"""IIS validation gate orchestrator (stdlib only)."""

from __future__ import annotations

import argparse
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time
import traceback
from dataclasses import asdict, dataclass


from check_mutation_flags import scan_source_for_true_flags


@dataclass
class StageResult:
    name: str
    status: str  # "pass" | "fail" | "skipped"
    exit_code: int
    duration_s: float
    log_path: str
    skip_reason: str = ""


def run_stage(
    name: str,
    argv: list[str],
    cwd: str,
    log_dir: str | None = None,
    timeout_s: float | None = None,
) -> StageResult:
    log_dir = log_dir or tempfile.gettempdir()
    pathlib.Path(log_dir).mkdir(parents=True, exist_ok=True)
    log_path = os.path.join(log_dir, f"iis_gate_{name}.log")
    start = time.monotonic()
    try:
        with open(log_path, "w", encoding="utf-8") as fh:
            proc = subprocess.run(
                argv,
                cwd=cwd,
                stdout=fh,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=timeout_s if timeout_s and timeout_s > 0 else None,
            )
        dur = time.monotonic() - start
        status = "pass" if proc.returncode == 0 else "fail"
        return StageResult(name, status, proc.returncode, round(dur, 2), log_path)
    except subprocess.TimeoutExpired as exc:
        dur = time.monotonic() - start
        with open(log_path, "a", encoding="utf-8") as fh:
            fh.write(f"\n[gate] stage timed out after {timeout_s}s\n")
            if exc.stdout:
                fh.write(exc.stdout)
            if exc.stderr:
                fh.write(exc.stderr)
        return StageResult(name, "fail", -2, round(dur, 2), log_path)
    except (OSError, subprocess.SubprocessError) as exc:
        dur = time.monotonic() - start
        with open(log_path, "w", encoding="utf-8") as fh:
            fh.write(f"[gate] failed to run stage: {exc!r}\n")
            fh.write(traceback.format_exc())
        return StageResult(name, "fail", -1, round(dur, 2), log_path)


def skipped_stage(name: str, reason: str) -> StageResult:
    return StageResult(name, "skipped", 0, 0.0, "", reason)


def run_mutation_flags_stage(
    name: str,
    scan_roots: list[str],
    log_dir: str,
) -> StageResult:
    pathlib.Path(log_dir).mkdir(parents=True, exist_ok=True)
    log_path = os.path.join(log_dir, f"iis_gate_{name}.log")
    start = time.monotonic()
    violations = scan_source_for_true_flags(scan_roots)
    dur = round(time.monotonic() - start, 2)
    with open(log_path, "w", encoding="utf-8") as fh:
        if violations:
            fh.write("[gate] mutation flag invariant violated:\n")
            for violation in violations:
                fh.write(violation + "\n")
        else:
            fh.write("[gate] no mutation flag violations found\n")
    if violations:
        return StageResult(name, "fail", 1, dur, log_path)
    return StageResult(name, "pass", 0, dur, log_path)


def build_report(results: list[StageResult], *, fail_on_skip: bool = False) -> dict:
    counts = {"pass": 0, "fail": 0, "skipped": 0}
    for r in results:
        counts[r.status] += 1
    overall = "pass"
    if counts["fail"] > 0:
        overall = "fail"
    elif fail_on_skip:
        blocking_skips = [
            r
            for r in results
            if r.status == "skipped" and r.skip_reason != "disabled in config"
        ]
        if blocking_skips:
            overall = "fail"
    return {
        "overall": overall,
        "counts": counts,
        "stages": [asdict(r) for r in results],
    }


def gate_repo_root() -> str:
    return os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")
    )


def _abs_path(base_dir: str, path: str) -> str:
    if os.path.isabs(path):
        return os.path.normpath(path)
    return os.path.normpath(os.path.join(base_dir, path))


def write_report(report: dict, out_path: str) -> None:
    pathlib.Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as fh:
        json.dump(report, fh, indent=2)


def report_to_markdown(report: dict) -> str:
    overall = report.get("overall", "unknown")
    title_verdict = overall.upper() if overall == "fail" else overall
    lines = [
        f"# IIS Release Gate — {title_verdict}",
        "",
        "| Stage | Status | Duration (s) | Log |",
        "|---|---|---|---|",
    ]
    for stage in report.get("stages", []):
        log_path = stage.get("log_path") or stage.get("skip_reason", "")
        lines.append(
            f"| {stage.get('name', '')} | {stage.get('status', '')} | "
            f"{stage.get('duration_s', '')} | {log_path} |"
        )
    lines.extend(["", f"Counts: {report.get('counts', {})}"])
    return "\n".join(lines)


def write_markdown_report(report: dict, out_path: str) -> None:
    pathlib.Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as fh:
        fh.write(report_to_markdown(report))


def stages_for_profile(profile: str) -> list[str]:
    base = [
        "buildplugin",
        "crossbuild",
        "editor_smoke",
        "automation",
        "pytest",
        "diff_check",
    ]
    if profile == "release":
        return base + ["automation_full", "sample_scan", "mutation_flags"]
    return base


def apply_profile_stages(stages: dict, profile: str) -> dict:
    """Enable profile stages without overriding explicit false in local config."""
    merged = dict(stages)
    for name in stages_for_profile(profile):
        if name not in merged:
            merged[name] = True
    return merged


def _stage_enabled(stages: dict, key: str) -> bool:
    return bool(stages.get(key, False))


def _plugins_root(cfg: dict) -> str:
    return cfg.get("plugins_root") or "AIPlugins"


def _test_project(cfg: dict) -> str:
    return cfg.get("test_project") or ""


def _transport_active(cfg: dict) -> bool:
    """Editor-dependent stages run against the transported test project when this is on."""
    return bool(cfg.get("_transport_active") and _test_project(cfg))


def _editor_project(cfg: dict) -> str:
    """The .uproject editor stages run against: transported test project, else the dev host."""
    return _test_project(cfg) if _transport_active(cfg) else cfg.get("project_file", "")


def _sample_project(cfg: dict) -> str:
    return _test_project(cfg) if _transport_active(cfg) else (cfg.get("sample_project") or "")


def transport_plugin(pkg_dir: str, test_project_dir: str, plugin: str) -> str:
    """Clean-copy a built plugin package into <test_project_dir>/Plugins/<plugin>; returns dest."""
    dest = os.path.join(test_project_dir, "Plugins", plugin)
    if os.path.isdir(dest):
        shutil.rmtree(dest)
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    shutil.copytree(pkg_dir, dest)
    return dest


def plugins_path_gitignored(test_project_dir: str, plugin: str) -> bool:
    """
    Guardrail: transported plugins may sit in Plugins/ but must NEVER be committable.
    True if the test project is not a git repo, or git itself reports Plugins/<plugin>
    as ignored. Uses `git check-ignore` (authoritative; respects nested rules and
    negations), falling back to a literal .gitignore scan only when git is unavailable.
    """
    if not os.path.isdir(os.path.join(test_project_dir, ".git")):
        return True  # no git => nothing to accidentally commit
    rel = os.path.join("Plugins", plugin)
    try:
        proc = subprocess.run(
            ["git", "-C", test_project_dir, "check-ignore", "-q", rel],
            capture_output=True,
        )
        if proc.returncode in (0, 1):
            return proc.returncode == 0  # 0 = ignored, 1 = not ignored
    except (OSError, ValueError):
        pass
    # Fallback: literal scan (git missing or .git is a stub).
    gitignore = os.path.join(test_project_dir, ".gitignore")
    if not os.path.isfile(gitignore):
        return False
    accepted = {
        "Plugins", "Plugins/", "/Plugins", "/Plugins/",
        f"Plugins/{plugin}", f"Plugins/{plugin}/",
        f"/Plugins/{plugin}", f"/Plugins/{plugin}/",
    }
    with open(gitignore, encoding="utf-8", errors="ignore") as fh:
        for raw in fh:
            line = raw.strip()
            if line and not line.startswith("#") and line in accepted:
                return True
    return False


def current_git_branch(test_project_dir: str) -> str | None:
    """Current branch of the test project, or None if it is not a resolvable git repo."""
    try:
        proc = subprocess.run(
            ["git", "-C", test_project_dir, "rev-parse", "--abbrev-ref", "HEAD"],
            capture_output=True,
            text=True,
        )
    except (OSError, ValueError):
        return None
    if proc.returncode != 0:
        return None
    return proc.stdout.strip() or None


def _argv_buildplugin(cfg: dict, plugin: str, repo_root: str) -> list[str]:
    uat = os.path.join(cfg["engine_root"], cfg["runuat"])
    uplugin = _abs_path(
        repo_root, os.path.join(_plugins_root(cfg), plugin, f"{plugin}.uplugin")
    )
    package = _abs_path(
        repo_root, os.path.join("Saved", "IISValidation", "pkg", plugin)
    )
    return [
        uat,
        "BuildPlugin",
        f"-Plugin={uplugin}",
        f"-Package={package}",
        "-TargetPlatforms=Win64",
    ]


def _argv_crossbuild(cfg: dict) -> list[str]:
    # Build.bat targets the game editor module only. RunUAT BuildEditor on UE 5.4
    # can pull invalid companion targets (e.g. UnrealPak) and fail spuriously.
    build_bat = os.path.join(
        cfg["engine_root"],
        cfg.get("build_batch", "Engine/Build/BatchFiles/Build.bat"),
    )
    project_file = os.path.normpath(cfg["project_file"])
    project_stem = os.path.splitext(os.path.basename(project_file))[0]
    return [
        build_bat,
        f"{project_stem}Editor",
        "Win64",
        "Development",
        f"-Project={project_file}",
        "-WaitMutex",
    ]


# A -nullrhi UnrealEditor has no viewport, so the "Quit" console command is a
# no-op and the editor idles forever (only EOS/online heartbeats), forcing the
# stage to its hard timeout. The reliable headless exit is -TestExit: the editor
# shuts down as soon as the automation controller logs this phrase, i.e. when the
# RunTests queue drains. -stdout streams the editor log into the gate stage log.
_TEST_EXIT = "-TestExit=Automation Test Queue Empty"
_EDITOR_FLAGS = ["-unattended", "-nullrhi", "-nopause", "-nosplash", "-stdout"]


def _argv_editor_smoke(cfg: dict) -> list[str]:
    editor = os.path.join(cfg["engine_root"], cfg["editor_cmd"])
    return [
        editor,
        _editor_project(cfg),
        "-ExecCmds=Automation RunTests InternalIndexService.Subsystem",
        _TEST_EXIT,
        *_EDITOR_FLAGS,
    ]


def _argv_automation(cfg: dict) -> list[str]:
    editor = os.path.join(cfg["engine_root"], cfg["editor_cmd"])
    return [
        editor,
        _editor_project(cfg),
        "-ExecCmds=Automation RunTests InternalIndexService",
        _TEST_EXIT,
        *_EDITOR_FLAGS,
    ]


def _argv_automation_full(cfg: dict) -> list[str]:
    editor = os.path.join(cfg["engine_root"], cfg["editor_cmd"])
    exec_cmds = (
        "Automation RunTests InternalIndexService.Incremental+"
        "InternalIndexService.UsageGraph+"
        "InternalIndexService.Blueprint+"
        "InternalIndexService.Editor+"
        "InternalIndexService.Mcp"
    )
    return [
        editor,
        _editor_project(cfg),
        f"-ExecCmds={exec_cmds}",
        _TEST_EXIT,
        *_EDITOR_FLAGS,
    ]


def _argv_sample_scan(cfg: dict) -> list[str]:
    editor = os.path.join(cfg["engine_root"], cfg["editor_cmd"])
    return [
        editor,
        _sample_project(cfg),
        "-ExecCmds=Automation RunTests InternalIndexService.Subsystem",
        _TEST_EXIT,
        *_EDITOR_FLAGS,
    ]


EDITOR_LAUNCH_STAGES = {
    "editor_smoke",
    "automation",
    "automation_full",
    "sample_scan",
}


def stage_timeout_for(cfg: dict, name: str) -> float:
    """Editor-launch stages get a bounded timeout so a wedged editor fails in
    minutes instead of hanging to the long build timeout; build/compile stages
    (buildplugin_*, crossbuild) keep the long stage_timeout_s."""
    if name in EDITOR_LAUNCH_STAGES:
        return float(cfg.get("editor_stage_timeout_s", 1800))
    return float(cfg.get("stage_timeout_s", 0))


def _check_strict(cfg: dict, stages: dict, strict: bool) -> None:
    if not strict:
        return
    missing = []
    if _stage_enabled(stages, "buildplugin") or _stage_enabled(stages, "crossbuild"):
        if not cfg.get("engine_root"):
            missing.append("engine_root (UE_ENGINE_ROOT / --engine-root)")
    if (
        _stage_enabled(stages, "crossbuild")
        or _stage_enabled(stages, "editor_smoke")
        or _stage_enabled(stages, "automation")
        or _stage_enabled(stages, "automation_full")
    ):
        if not cfg.get("project_file"):
            missing.append("project_file (UE_PROJECT_FILE / --project-file)")
    if _stage_enabled(stages, "sample_scan") and not cfg.get("sample_project"):
        missing.append("sample_project (gate_config sample_project)")
    if _stage_enabled(stages, "transport") and not cfg.get("test_project"):
        missing.append("test_project (transport stage target .uproject)")
    if missing:
        print(
            "iis_gate --strict: required configuration missing:\n  - "
            + "\n  - ".join(missing),
            file=sys.stderr,
        )
        raise SystemExit(2)


def main() -> None:
    ap = argparse.ArgumentParser(description="IIS multi-stage validation gate")
    ap.add_argument(
        "--config",
        default=os.path.join(os.path.dirname(__file__), "gate_config.json"),
    )
    ap.add_argument("--engine-root", default=os.environ.get("UE_ENGINE_ROOT", ""))
    ap.add_argument("--project-file", default=os.environ.get("UE_PROJECT_FILE", ""))
    ap.add_argument(
        "--profile",
        choices=["default", "release"],
        default="default",
        help="Gate profile: release adds automation_full, sample_scan, mutation_flags",
    )
    ap.add_argument(
        "--report",
        default=None,
        help="Output JSON path (default depends on --profile)",
    )
    ap.add_argument("--fail-fast", action="store_true")
    ap.add_argument(
        "--strict",
        action="store_true",
        help="Exit 2 if enabled stages need engine_root/project_file but they are unset",
    )
    args = ap.parse_args()

    if args.report is None:
        if args.profile == "release":
            args.report = os.path.join(
                "Saved", "IISValidation", "release_gate_report.json"
            )
        else:
            args.report = os.path.join("Saved", "IISValidation", "gate_report.json")

    results: list[StageResult] = []
    report_path = args.report

    try:
        with open(args.config, encoding="utf-8") as fh:
            cfg = json.load(fh)
        if args.engine_root:
            cfg["engine_root"] = args.engine_root
        if args.project_file:
            cfg["project_file"] = args.project_file

        stages = apply_profile_stages(cfg.get("stages", {}), args.profile)
        cfg["_transport_active"] = bool(
            _stage_enabled(stages, "transport") and _test_project(cfg)
        )
        _check_strict(cfg, stages, args.strict)

        log_dir = os.path.join("Saved", "IISValidation", "logs")

        def maybe(
            name: str,
            enabled: bool,
            argv: list[str],
            *,
            engine_dependent: bool = False,
            editor_dependent: bool = False,
            sample_project_dependent: bool = False,
            stage_cwd: str = ".",
        ) -> bool:
            if not enabled:
                results.append(skipped_stage(name, "disabled in config"))
                return True
            if engine_dependent and not cfg.get("engine_root"):
                results.append(skipped_stage(name, "no engine_root"))
                return True
            if editor_dependent and not cfg.get("project_file"):
                results.append(skipped_stage(name, "no project_file"))
                return True
            if sample_project_dependent and not cfg.get("sample_project"):
                results.append(skipped_stage(name, "no sample_project"))
                return True
            res = run_stage(
                name,
                argv,
                cwd=stage_cwd,
                log_dir=log_dir,
                timeout_s=stage_timeout_for(cfg, name),
            )
            results.append(res)
            return res.status != "fail"

        repo_root = gate_repo_root()
        os.chdir(repo_root)

        ok = True
        for plugin in cfg.get("plugins", []):
            ok = maybe(
                f"buildplugin_{plugin}",
                _stage_enabled(stages, "buildplugin"),
                _argv_buildplugin(cfg, plugin, repo_root),
                engine_dependent=True,
            )
            if not ok and args.fail_fast:
                break

        mcp_proxy_dir = os.path.join(
            repo_root,
            _plugins_root(cfg),
            "InternalIndexService",
            "Tools",
            "mcp_proxy",
        )

        if ok or not args.fail_fast:
            if _stage_enabled(stages, "transport"):
                test_proj = _test_project(cfg)
                if not test_proj:
                    results.append(skipped_stage("transport", "no test_project"))
                else:
                    test_dir = os.path.dirname(os.path.normpath(test_proj))
                    transport_log = os.path.join(log_dir, "iis_gate_transport.log")
                    lines: list[str] = []
                    transport_ok = True
                    want_branch = cfg.get("test_branch") or ""
                    cur_branch = current_git_branch(test_dir)
                    if want_branch and cur_branch is not None and cur_branch != want_branch:
                        lines.append(
                            f"REFUSED - test project is on branch '{cur_branch}', "
                            f"expected '{want_branch}'; checkout the product branch in "
                            f"{test_dir} before the gate (one branch per plugin)"
                        )
                        transport_ok = False
                    else:
                        for plugin in cfg.get("plugins", []):
                            pkg = _abs_path(
                                repo_root,
                                os.path.join("Saved", "IISValidation", "pkg", plugin),
                            )
                            if not os.path.isdir(pkg):
                                lines.append(f"{plugin}: FAIL package missing ({pkg})")
                                transport_ok = False
                                continue
                            if not plugins_path_gitignored(test_dir, plugin):
                                lines.append(
                                    f"{plugin}: REFUSED - {test_dir}/Plugins is not "
                                    f"git-ignored; add 'Plugins/' to .gitignore "
                                    f"(plugins must never be committed)"
                                )
                                transport_ok = False
                                continue
                            dest = transport_plugin(pkg, test_dir, plugin)
                            lines.append(f"{plugin}: OK -> {dest}")
                    os.makedirs(log_dir, exist_ok=True)
                    with open(transport_log, "w", encoding="utf-8") as fh:
                        fh.write("\n".join(lines) + "\n")
                    results.append(
                        StageResult(
                            "transport",
                            "pass" if transport_ok else "fail",
                            0 if transport_ok else 1,
                            0.0,
                            transport_log,
                        )
                    )
                    if not transport_ok:
                        ok = False

            stage_defs: list[tuple] = [
                (
                    "crossbuild",
                    _stage_enabled(stages, "crossbuild"),
                    _argv_crossbuild(cfg),
                    True,
                    True,
                    False,
                    ".",
                ),
                (
                    "editor_smoke",
                    _stage_enabled(stages, "editor_smoke"),
                    _argv_editor_smoke(cfg),
                    True,
                    True,
                    False,
                    ".",
                ),
                (
                    "automation",
                    _stage_enabled(stages, "automation"),
                    _argv_automation(cfg),
                    True,
                    True,
                    False,
                    ".",
                ),
                (
                    "automation_full",
                    _stage_enabled(stages, "automation_full"),
                    _argv_automation_full(cfg),
                    True,
                    True,
                    False,
                    ".",
                ),
                (
                    "sample_scan",
                    _stage_enabled(stages, "sample_scan"),
                    _argv_sample_scan(cfg),
                    True,
                    False,
                    True,
                    ".",
                ),
                (
                    "pytest_mcp_proxy",
                    _stage_enabled(stages, "pytest"),
                    [sys.executable, "-m", "pytest", "tests", "-q"],
                    False,
                    False,
                    False,
                    mcp_proxy_dir,
                ),
                (
                    "diff_check",
                    _stage_enabled(stages, "diff_check"),
                    ["git", "diff", "--check"],
                    False,
                    False,
                    False,
                    repo_root,
                ),
            ]
            for name, enabled, argv, eng_dep, ed_dep, sample_dep, stage_cwd in stage_defs:
                if not ok and args.fail_fast:
                    break
                ok = maybe(
                    name,
                    enabled,
                    argv,
                    engine_dependent=eng_dep,
                    editor_dependent=ed_dep,
                    sample_project_dependent=sample_dep,
                    stage_cwd=stage_cwd,
                )
                if not ok and args.fail_fast:
                    break

            if ok or not args.fail_fast:
                if _stage_enabled(stages, "mutation_flags"):
                    scan_root = os.path.join(
                        repo_root,
                        _plugins_root(cfg),
                        "InternalIndexService",
                        "Source",
                    )
                    res = run_mutation_flags_stage(
                        "mutation_flags", [scan_root], log_dir
                    )
                    results.append(res)
                    if res.status == "fail":
                        ok = False
                else:
                    results.append(
                        skipped_stage("mutation_flags", "disabled in config")
                    )
    except Exception as exc:
        results.append(
            StageResult(
                "gate_orchestrator",
                "fail",
                -1,
                0.0,
                "",
                skip_reason=str(exc),
            )
        )
        report = build_report(results, fail_on_skip=args.profile == "release")
        write_report(report, report_path)
        if args.profile == "release":
            md_path = os.path.splitext(report_path)[0] + ".md"
            write_markdown_report(report, md_path)
        print(json.dumps(report["counts"]))
        raise SystemExit(1) from exc

    report = build_report(results, fail_on_skip=args.profile == "release")
    write_report(report, report_path)
    if args.profile == "release":
        md_path = os.path.splitext(report_path)[0] + ".md"
        write_markdown_report(report, md_path)
    print(json.dumps(report["counts"]))
    raise SystemExit(0 if report["overall"] == "pass" else 1)


if __name__ == "__main__":
    main()
