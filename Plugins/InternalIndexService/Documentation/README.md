# Internal Index Service

Internal Index Service (IIS) is an Unreal Engine plugin for **local** indexing, retrieval, context packs, and agent-facing search. It is retrieval-only: it does not mutate project content or generate patches.

## Modules

| Module | Purpose |
| --- | --- |
| `InternalIndexServiceInterface` | Public contracts for chunks, search, imports, embeddings, and agent access. |
| `InternalIndexService` | Runtime engine subsystem, chunk catalog, vector backends, embedding jobs, incremental indexing. |
| `InternalIndexServiceEditor` | Editor control panel, MCP loopback endpoint, Python bridge, and Tools menu entries. |

## Open the Control Panel

```text
Tools -> Open Internal Index Service
```

Or use the nomad tab **Internal Index Service** if your layout already shows it.

The panel is organized as:

| Tab | Purpose |
| --- | --- |
| **Dashboard** | Health cards for Index, MCP Server, and Integrations; quick actions (Rebuild catalog, Start/Stop MCP). |
| **Overview** | Service version, index root, catalog/vector presence, MCP summary. |
| **Imports** | UII handoff and import report summaries. |
| **Catalog** | Build catalog and `catalog_build_report.json` summary. |
| **Embeddings** | Build and execute embedding jobs (routes through LLM Store). |
| **Search** | Inline lexical / vector / hybrid search against the local index. |
| **Agent/MCP** | Agent contracts, MCP tool manifest, MCP status. |
| **Reports** | Reports and logs folder shortcuts. |

The header shows the project name, a retrieval-only marker, a **Settings** button, and a **Refresh** button. Refresh updates all tab summaries, forces a Dashboard card update, and refreshes the Governance panel.

Dashboard cards are refreshed on open, by explicit **Refresh**, and after dashboard actions such as **Start**, **Stop**, or **Rebuild**. The panel does not rebuild cards on a periodic timer, which keeps the UI stable while editing or inspecting the panel.

## Project Settings

Configure IIS under:

```text
Edit -> Project Settings -> Plugins -> Internal Index Service
```

| Setting | Default | Meaning |
| --- | --- | --- |
| **Vector Backend** | `jsonl_bruteforce` | Local vector index implementation (`jsonl_bruteforce` or `hnsw`). |
| **Enable MCP Endpoint** | off | Start the loopback MCP server when the editor module loads. |
| **MCP Port** | `8731` | TCP port for `127.0.0.1` MCP (agents on the same machine only). |

MCP enable and port are also used by the editor MCP endpoint and the dashboard MCP card.

## MCP Endpoint

When enabled, IIS exposes a loopback MCP endpoint for AI agents. Use **Start** / **Stop** on the Dashboard MCP card or rely on auto-start when **Enable MCP Endpoint** is on in project settings.

- Bind address: `127.0.0.1` only (not exposed to the network).
- Session token: written to a handshake file; rotate via editor module API (Governance UI in a later panel version).
- External stdio proxy: `Plugins/InternalIndexService/Tools/mcp_proxy` (see `Tools/mcp_proxy/README.md`).

## Integrations

Embedding execution is delegated to **LLM Store** through `FIISEmbeddingRouteExecutorRegistry`. The optional **InternalIndexServiceLLMStoreBridge** plugin registers the `llmstore` executor when both IIS and LLM Store are enabled.

The Dashboard **Integrations** card lists registered executor IDs and whether each bridge is active.

## Validation Gate

Before release or large merges, run the IIS validation gate:

```text
Plugins/InternalIndexService/Tools/iis_gate/run_iis_gate.py
```

See [docs/architecture/iis_validation_gate.md](../../../docs/architecture/iis_validation_gate.md) for profiles (`default` vs `release`), local config, and stage list.

## Related Documentation

| Document | Audience |
| --- | --- |
| [CONTROL_PANEL.md](CONTROL_PANEL.md) | Editor panel workflows (dashboard, tabs, MCP). |
| [SETTINGS.md](SETTINGS.md) | `UIISSettings` fields and migration from manual config. |
| [THIRD_PARTY_SOFTWARE.md](THIRD_PARTY_SOFTWARE.md) | Third-party source declaration details for Fab submission. |
| [../CHANGELOG.md](../CHANGELOG.md) | Release notes. |
| [../../../docs/architecture/iis_validation_gate.md](../../../docs/architecture/iis_validation_gate.md) | CI / release gate. |
| [../../../docs/architecture/uII_iis_llmstore_boundary.md](../../../docs/architecture/uII_iis_llmstore_boundary.md) | UII / IIS / LLM Store ownership. |
| [../../../docs/architecture/llmstore_embedding_routes.md](../../../docs/architecture/llmstore_embedding_routes.md) | Embedding route contract. |

## Packaging

Ensure `Config/FilterPlugin.ini` includes `/Documentation/...` and `/Tools/...` so buyers receive this folder and the validation/proxy tooling in Fab builds. Keep local gate config and Python cache files excluded from packaged builds.
