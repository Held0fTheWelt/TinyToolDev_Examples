#!/usr/bin/env python3
"""Validate IIS MVP-E agent access surfaces (contracts, manifest, optional live MCP).

Run after the editor has written agent contracts (Tools menu or Control Panel) or
with --contracts-dir pointing at Saved/InternalIndexService/agent_contracts.

Exit codes: 0 = pass, 1 = validation failure, 2 = usage/config error.
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.error
import urllib.request
from datetime import datetime, timezone
from pathlib import Path

EXPECTED_TOOLS = sorted(
    [
        "iis_explain_blueprint",
        "iis_find_usages",
        "iis_get_chunk",
        "iis_get_context_pack",
        "iis_get_source_references",
        "iis_search",
    ]
)


def read_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def resolve_contracts_dir(project_dir: Path, explicit: str | None) -> Path:
    if explicit:
        return Path(explicit).expanduser().resolve()
    return (project_dir / "Saved" / "InternalIndexService" / "agent_contracts").resolve()


def validate_contract_files(contracts_dir: Path) -> dict:
    result: dict = {"contracts_dir": str(contracts_dir), "files_present": {}}
    for name in (
        "iis_agent_tool_contracts.json",
        "iis_agent_tool_registry.json",
        "mcp_tool_manifest.json",
    ):
        path = contracts_dir / name
        result["files_present"][name] = path.is_file()

    manifest_path = contracts_dir / "mcp_tool_manifest.json"
    if manifest_path.is_file():
        manifest = read_json(manifest_path)
        names = sorted(t.get("name", "") for t in manifest.get("tools", []))
        result["manifest_tool_names"] = names
        result["manifest_tool_count_ok"] = names == EXPECTED_TOOLS
    else:
        result["manifest_tool_names"] = []
        result["manifest_tool_count_ok"] = False

    handshake_path = contracts_dir / "mcp_endpoint.json"
    result["handshake_present"] = handshake_path.is_file()
    if handshake_path.is_file():
        result["handshake"] = read_json(handshake_path)

    return result


def validate_mcp_health(handshake: dict) -> dict:
    host = handshake.get("host", "127.0.0.1")
    port = handshake.get("port", 8731)
    token = handshake.get("token", "")
    url = f"http://{host}:{port}/mcp/health"
    request = urllib.request.Request(url, headers={"Authorization": f"Bearer {token}"})
    try:
        with urllib.request.urlopen(request, timeout=5) as response:
            body = response.read().decode("utf-8")
            payload = json.loads(body)
            return {
                "health_url": url,
                "health_ok": payload.get("status") in ("ready", "Ready"),
                "health_payload": payload,
            }
    except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as exc:
        return {"health_url": url, "health_ok": False, "health_error": str(exc)}


def overall_pass(summary: dict, require_health: bool) -> bool:
    contracts = summary.get("contracts", {})
    files_ok = all(contracts.get("files_present", {}).values())
    manifest_ok = contracts.get("manifest_tool_count_ok", False)
    if require_health:
        return files_ok and manifest_ok and summary.get("live_endpoint", {}).get("health_ok", False)
    return files_ok and manifest_ok


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate IIS MVP-E agent surfaces.")
    parser.add_argument(
        "--project-dir",
        default=".",
        help="Unreal project directory (default: cwd).",
    )
    parser.add_argument(
        "--contracts-dir",
        default=None,
        help="Override agent_contracts directory (default: <project>/Saved/InternalIndexService/agent_contracts).",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Write JSON summary here (default: <project>/Saved/IISValidation/validate_iis_mvp_e_summary.json).",
    )
    parser.add_argument(
        "--check-health",
        action="store_true",
        help="Require live MCP /mcp/health when mcp_endpoint.json exists.",
    )
    args = parser.parse_args()

    project_dir = Path(args.project_dir).expanduser().resolve()
    contracts_dir = resolve_contracts_dir(project_dir, args.contracts_dir)
    if not contracts_dir.is_dir():
        print(f"Contracts directory not found: {contracts_dir}", file=sys.stderr)
        print("Open the editor, enable MCP if needed, and run 'Write Agent Tool Contracts'.", file=sys.stderr)
        return 2

    summary = {
        "validated_at": datetime.now(timezone.utc).isoformat(),
        "project_dir": str(project_dir),
        "expected_tools": EXPECTED_TOOLS,
        "contracts": validate_contract_files(contracts_dir),
        "live_endpoint": {},
    }

    handshake = summary["contracts"].get("handshake")
    if handshake:
        summary["live_endpoint"] = validate_mcp_health(handshake)
    elif args.check_health:
        summary["live_endpoint"] = {"health_ok": False, "health_error": "mcp_endpoint.json missing"}

    output_path = (
        Path(args.output).expanduser().resolve()
        if args.output
        else project_dir / "Saved" / "IISValidation" / "validate_iis_mvp_e_summary.json"
    )
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    passed = overall_pass(summary, require_health=args.check_health)
    print(json.dumps(summary, indent=2))
    print(f"Summary written: {output_path}")
    print("PASS" if passed else "FAIL")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
