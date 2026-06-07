import os
import ntpath
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))

from run_iis_gate import (
    StageResult,
    _argv_automation,
    _argv_automation_full,
    _argv_buildplugin,
    _argv_crossbuild,
    _argv_editor_smoke,
    _argv_sample_scan,
    _editor_project,
    build_report,
    plugins_path_gitignored,
    report_to_markdown,
    run_stage,
    skipped_stage,
    stage_timeout_for,
    stages_for_profile,
    transport_plugin,
)
from check_mutation_flags import scan_source_for_true_flags


def _is_absolute_path(path: str) -> bool:
    return os.path.isabs(path) or ntpath.isabs(path)


def _endswith_path_parts(path: str, *parts: str) -> bool:
    return path.replace("\\", "/").endswith("/".join(parts))


def test_transport_plugin_clean_copies(tmp_path):
    pkg = tmp_path / "pkg" / "InternalIndexService"
    (pkg / "Source").mkdir(parents=True)
    (pkg / "InternalIndexService.uplugin").write_text("{}")
    (pkg / "Source" / "x.cpp").write_text("// x")
    test_proj_dir = tmp_path / "TestProject"
    test_proj_dir.mkdir()

    dest = transport_plugin(str(pkg), str(test_proj_dir), "InternalIndexService")
    assert os.path.isfile(os.path.join(dest, "InternalIndexService.uplugin"))
    assert os.path.isfile(os.path.join(dest, "Source", "x.cpp"))
    assert dest == os.path.join(str(test_proj_dir), "Plugins", "InternalIndexService")

    # Idempotent: a second transport cleanly replaces (no stale files).
    (pkg / "Source" / "y.cpp").write_text("// y")
    dest2 = transport_plugin(str(pkg), str(test_proj_dir), "InternalIndexService")
    assert os.path.isfile(os.path.join(dest2, "Source", "y.cpp"))


def test_plugins_must_be_gitignored_in_a_git_test_project(tmp_path):
    import subprocess

    proj = tmp_path / "TestProject"
    proj.mkdir()
    subprocess.run(["git", "init", "-q", str(proj)], check=True)
    # Real git repo, no .gitignore => unsafe (could be committed).
    assert plugins_path_gitignored(str(proj), "InternalIndexService") is False
    # The TinyToolDev_Examples rule: a bare '/Plugins' ignores the whole folder.
    (proj / ".gitignore").write_text("Saved/\nIntermediate/\n/Plugins\n")
    assert plugins_path_gitignored(str(proj), "InternalIndexService") is True
    assert plugins_path_gitignored(str(proj), "SmartContentDiet") is True


def test_non_git_test_project_is_allowed(tmp_path):
    proj = tmp_path / "PlainProject"
    proj.mkdir()
    # No .git => nothing to accidentally commit.
    assert plugins_path_gitignored(str(proj), "InternalIndexService") is True


def test_current_git_branch_none_for_non_repo(tmp_path):
    from run_iis_gate import current_git_branch

    proj = tmp_path / "NotARepo"
    proj.mkdir()
    assert current_git_branch(str(proj)) is None


def test_editor_project_uses_test_project_under_transport():
    cfg = {
        "project_file": "/dev/PluginProject.uproject",
        "test_project": "/clean/TestProject.uproject",
        "_transport_active": True,
    }
    assert _editor_project(cfg) == "/clean/TestProject.uproject"
    cfg["_transport_active"] = False
    assert _editor_project(cfg) == "/dev/PluginProject.uproject"


def test_automation_targets_test_project_under_transport():
    cfg = {
        "engine_root": "/fake/ue",
        "editor_cmd": "Engine/Binaries/Linux/UnrealEditor-Cmd",
        "project_file": "/dev/PluginProject.uproject",
        "test_project": "/clean/TestProject.uproject",
        "_transport_active": True,
    }
    argv = _argv_automation(cfg)
    assert "/clean/TestProject.uproject" in argv
    assert "/dev/PluginProject.uproject" not in argv


def test_release_profile_supersets_default():
    base = set(stages_for_profile("default"))
    rel = set(stages_for_profile("release"))
    assert base.issubset(rel)
    assert {"sample_scan", "mutation_flags", "automation_full"} <= rel


def test_automation_full_covers_epic_test_filters():
    cfg = {
        "engine_root": "/fake/ue",
        "editor_cmd": "Engine/Binaries/Linux/UnrealEditor-Cmd",
        "project_file": "/fake/PluginProject.uproject",
    }
    argv = _argv_automation_full(cfg)
    exec_cmds = next(arg for arg in argv if arg.startswith("-ExecCmds="))
    assert "InternalIndexService.Incremental" in exec_cmds
    assert "InternalIndexService.UsageGraph" in exec_cmds
    assert "InternalIndexService.Blueprint" in exec_cmds
    assert "InternalIndexService.Editor" in exec_cmds
    assert "InternalIndexService.Mcp" in exec_cmds


def test_detects_a_true_mutation_flag(tmp_path):
    f = tmp_path / "X.cpp"
    f.write_text("bAllowsProjectMutation = true;", encoding="utf-8")
    violations = scan_source_for_true_flags([str(tmp_path)])
    assert any("bAllowsProjectMutation" in v for v in violations)


def test_clean_source_has_no_violations(tmp_path):
    f = tmp_path / "Y.cpp"
    f.write_text("bAllowsProjectMutation = false;", encoding="utf-8")
    assert scan_source_for_true_flags([str(tmp_path)]) == []


def test_markdown_lists_all_stages_and_verdict():
    rep = build_report(
        [
            StageResult("buildplugin_IIS", "pass", 0, 3.0, "/l/a.log"),
            StageResult("mutation_flags", "fail", 1, 0.2, "/l/b.log"),
        ]
    )
    md = report_to_markdown(rep)
    assert "buildplugin_IIS" in md and "mutation_flags" in md
    assert "FAIL" in md.upper()


def test_run_stage_records_pass_on_zero_exit():
    res = run_stage("noop", [sys.executable, "-c", "print('ok')"], cwd=".")
    assert isinstance(res, StageResult)
    assert res.status == "pass"
    assert res.exit_code == 0
    assert res.log_path


def test_run_stage_records_fail_on_nonzero_exit():
    res = run_stage(
        "boom",
        [sys.executable, "-c", "import sys; sys.exit(3)"],
        cwd=".",
    )
    assert res.status == "fail"
    assert res.exit_code == 3


def test_build_report_overall_fail_if_any_fail():
    results = [
        StageResult("a", "pass", 0, 1.0, "/x/a.log"),
        StageResult("b", "fail", 2, 1.0, "/x/b.log"),
        StageResult("c", "skipped", 0, 0.0, ""),
    ]
    report = build_report(results)
    assert report["overall"] == "fail"
    assert report["stages"][1]["name"] == "b"
    assert report["counts"] == {"pass": 1, "fail": 1, "skipped": 1}


def test_build_report_overall_pass_when_no_fail():
    results = [
        StageResult("a", "pass", 0, 1.0, "/x/a.log"),
        StageResult("c", "skipped", 0, 0.0, ""),
    ]
    assert build_report(results)["overall"] == "pass"


def test_release_profile_fails_when_stages_skipped():
    results = [StageResult("crossbuild", "skipped", 0, 0.0, "", "no engine_root")]
    assert build_report(results, fail_on_skip=True)["overall"] == "fail"


def test_apply_profile_stages_respects_explicit_false():
    from run_iis_gate import apply_profile_stages

    merged = apply_profile_stages({"crossbuild": False, "transport": True}, "release")
    assert merged["crossbuild"] is False
    assert merged["automation_full"] is True
    assert merged["mutation_flags"] is True


def test_release_profile_allows_config_disabled_skips():
    results = [
        StageResult("crossbuild", "skipped", 0, 0.0, "", "disabled in config"),
        StageResult("automation_full", "pass", 0, 1.0, "/x/a.log"),
    ]
    assert build_report(results, fail_on_skip=True)["overall"] == "pass"

def test_buildplugin_uses_absolute_plugin_and_package_paths():
    cfg = {
        "engine_root": "D:/Engines/UE_5.4",
        "runuat": "Engine/Build/BatchFiles/RunUAT.bat",
        "plugins_root": "AIPlugins",
    }
    repo_root = "D:/TinyToolDevelopment/Git"
    argv = _argv_buildplugin(cfg, "InternalIndexService", repo_root)
    plugin_arg = argv[2].split("=", 1)[1]
    package_arg = argv[3].split("=", 1)[1]
    assert _is_absolute_path(plugin_arg)
    assert _endswith_path_parts(
        plugin_arg,
        "AIPlugins",
        "InternalIndexService",
        "InternalIndexService.uplugin",
    )
    assert _is_absolute_path(package_arg)
    assert _endswith_path_parts(
        package_arg,
        "Saved",
        "IISValidation",
        "pkg",
        "InternalIndexService",
    )


def test_crossbuild_uses_build_bat_not_runuat_buildeditor():
    cfg = {
        "engine_root": "D:/Engines/UE_5.4",
        "project_file": "D:/TinyToolDevelopment/Git/PluginProject.uproject",
    }
    argv = _argv_crossbuild(cfg)
    assert argv[0].endswith("Build.bat")
    assert argv[1] == "PluginProjectEditor"
    assert "-Project=" in argv[4]


def test_skipped_stage_records_reason():
    res = skipped_stage("crossbuild", "no engine_root")
    assert res.status == "skipped"
    assert res.skip_reason == "no engine_root"
    report = build_report([res])
    assert report["stages"][0]["skip_reason"] == "no engine_root"


def test_run_stage_catches_missing_executable(tmp_path):
    res = run_stage(
        "missing_bin",
        [str(tmp_path / "does-not-exist-ue-binary")],
        cwd=str(tmp_path),
    )
    assert res.status == "fail"
    assert res.exit_code == -1
    assert res.log_path


def test_diff_check_detects_trailing_whitespace(tmp_path):
    res = run_stage(
        "diff_check",
        [sys.executable, "-c", "import sys; sys.exit(2)"],
        cwd=str(tmp_path),
    )
    assert res.status == "fail"
    assert res.exit_code == 2


def _editor_cfg():
    return {
        "engine_root": "/fake/ue",
        "editor_cmd": "Engine/Binaries/Linux/UnrealEditor-Cmd",
        "project_file": "/dev/PluginProject.uproject",
        "sample_project": "/dev/PluginProject.uproject",
    }


def test_editor_stages_exit_via_testexit_and_stream_stdout():
    # A -nullrhi UnrealEditor never exits from the "Quit" console command
    # (no viewport to close); it idles forever and the stage times out.
    # The reliable headless exit is -TestExit firing when the automation
    # queue drains. Every editor-launch stage must use it.
    for build in (
        _argv_editor_smoke,
        _argv_automation,
        _argv_automation_full,
        _argv_sample_scan,
    ):
        argv = build(_editor_cfg())
        joined = " ".join(argv)
        assert "-TestExit=Automation Test Queue Empty" in argv, build.__name__
        assert "-ExecCmds=Quit" not in argv, build.__name__
        assert "Quit" not in joined, build.__name__
        assert "-stdout" in argv, build.__name__


def test_automation_stages_still_run_iis_filters():
    assert "InternalIndexService" in " ".join(_argv_automation(_editor_cfg()))
    full = " ".join(_argv_automation_full(_editor_cfg()))
    for suite in ("Incremental", "UsageGraph", "Blueprint", "Editor", "Mcp"):
        assert f"InternalIndexService.{suite}" in full


def test_editor_launch_stages_get_shorter_timeout():
    cfg = {"stage_timeout_s": 7200, "editor_stage_timeout_s": 1800}
    for name in ("editor_smoke", "automation", "automation_full", "sample_scan"):
        assert stage_timeout_for(cfg, name) == 1800, name
    # Build/compile stages keep the long timeout (a cold crossbuild is slow).
    assert stage_timeout_for(cfg, "buildplugin_InternalIndexService") == 7200
    assert stage_timeout_for(cfg, "crossbuild") == 7200


def test_editor_stage_timeout_has_a_safe_default():
    # Even with no config, an editor launch must not inherit an unbounded wait.
    assert stage_timeout_for({}, "automation") == 1800
    assert stage_timeout_for({"stage_timeout_s": 0}, "diff_check") == 0
