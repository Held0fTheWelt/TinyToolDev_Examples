# Project Settings (`UIISSettings`)

IIS configuration lives in a standard Unreal **Developer Settings** class so teams can edit values in Project Settings instead of hand-editing `DefaultEngine.ini`.

**Path:** `Edit -> Project Settings -> Plugins -> Internal Index Service`

**Class:** `UIISSettings` (config = Engine, category Plugins)

## Fields

| Property | Config key | Default | Description |
| --- | --- | --- | --- |
| Vector Backend | (in-memory / future key) | `jsonl_bruteforce` | Selects the vector index backend. Supported values include `jsonl_bruteforce` and `hnsw` (lowercased at resolve time). |
| Enable MCP Endpoint | `bEnableMcpEndpoint` | `false` | When true, the editor module attempts to start the loopback MCP server on load. |
| MCP Port | `McpPort` | `8731` | Port for `127.0.0.1`. Invalid values fall back to `8731`. |
| Index Root (optional) | `IndexRoot` | *(empty)* | When set, overrides the on-disk index root. Empty uses `Saved/InternalIndexService`. |

## Consumers

These read sites use `GetDefault<UIISSettings>()`:

- `IISResolveConfiguredVectorBackendId()` / `IISVectorBackendFactory`
- `FIISMcpEndpoint::IsEnabledByConfig()` and `GetConfiguredPort()`
- Control Panel overview and dashboard MCP card (via `GetMcpStatus()`)

## Secrets and index paths

API keys for cloud embedding providers remain in **LLM Store** secret storage, not in IIS settings.

Index root and artifact paths are managed by `FIISStoragePaths`, which reads `UIISSettings::IndexRoot` when non-empty.

## Migration note

Older builds read MCP and vector settings directly from `GEngineIni`. After upgrading, open Project Settings once and save defaults so the Engine config section reflects your intended MCP port and backend. Behavior matches previous defaults: MCP off, port `8731`, vector backend `jsonl_bruteforce`.
