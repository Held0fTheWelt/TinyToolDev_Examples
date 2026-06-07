# IIS MVP-E validation

`validate_iis_mvp_e.py` checks that IIS agent access artifacts exist and that the MCP
tool manifest declares all six agent tools.

## Prerequisites

1. Open the host project in the Unreal Editor with **Internal Index Service** enabled.
2. Run **Tools → Internal Index Service → Write Agent Tool Contracts** (or use the
   Control Panel equivalent) so `Saved/InternalIndexService/agent_contracts/` is populated.
3. Optional live check: enable MCP in **Project Settings → Plugins → Internal Index Service**
   and start the endpoint from the Control Panel dashboard.

## Run

From the git monorepo root:

```powershell
cd D:\TinyToolDevelopment\Git
python AIPlugins/InternalIndexService/Tools/mvp_e_validation/validate_iis_mvp_e.py `
  --project-dir D:\TinyToolDevelopment\Git
```

With live endpoint health (requires editor MCP running):

```powershell
python AIPlugins/InternalIndexService/Tools/mvp_e_validation/validate_iis_mvp_e.py `
  --project-dir D:\TinyToolDevelopment\Git `
  --check-health
```

## Output

- Console: JSON summary + `PASS` / `FAIL`
- File: `Saved/IISValidation/validate_iis_mvp_e_summary.json` (gitignored under `Saved/`)

## Expected tools

`iis_search`, `iis_get_context_pack`, `iis_get_chunk`, `iis_get_source_references`,
`iis_find_usages`, `iis_explain_blueprint`

## Related

- Plan: `docs/superpowers/plans/2026-06-01-iis-mvp-e-agent-access.md` (Task 7)
- Boundary doc: `docs/architecture/uII_iis_llmstore_boundary.md`
- MCP proxy: `AIPlugins/InternalIndexService/Tools/mcp_proxy/`
