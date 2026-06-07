# Control Panel

The IIS Control Panel is the primary operator surface for indexing, search, embeddings, MCP, governance, and diagnostics.

Open it from **Tools → Internal Index Service → Open Internal Index Service**.

## Header

- **Title**, **project name**, and **retrieval-only marker** — reminds operators that IIS does not apply project mutations.
- **Settings** — jumps to the in-panel Settings tab.
- **Refresh** — reloads tab bodies, forces a Dashboard card update, and refreshes the Governance panel from a fresh `IISCapturePanelStatus()` snapshot.

Dashboard cards are refreshed on open, by explicit **Refresh**, and after dashboard actions such as **Start**, **Stop**, or **Rebuild**. The panel does not rebuild cards on a periodic timer, so volatile counters do not cause visible card flicker.

## Tabs

| Tab | Content |
| --- | --- |
| Dashboard | Health cards (Index, MCP, Integrations) plus work shortcuts (Import, quick Search, Data & Privacy). |
| Index | Imports, Catalog, Embeddings (scrollable). |
| Use | Search (lexical / vector / hybrid). |
| Agents | Agent/MCP tools and output folders. |
| Governance | Retrieval-only guarantee, MCP security, data locality, bridge health, agent tool transparency. |
| Diagnostics | Overview and Reports (scrollable). |
| Settings | In-panel `UIISSettings` details view and link to Project Settings. |

Tab buttons use **active-state** styling (highlighted tab vs dimmed siblings).

## Dashboard

### Health row

- **Index** — catalog/vector presence, chunk count; **Rebuild** runs `BuildChunkCatalogWithWarnings`.
- **MCP Server** — loopback address, token, request count; **Start** / **Stop**.
- **Integrations** — embedding executors from `FIISEmbeddingRouteExecutorRegistry`.

### Work row

- **Import** — **Go to Imports** switches to the Index tab.
- **Search** — inline query + **Go** runs search on the Use tab.
- **Data & Privacy** — **Open folder** opens the index root.

## Governance

Read-only display of mutation flags (default `FIISAgentToolResponse`), MCP loopback posture, index paths, bridge/executor registration (including LLM Store bridge load hints), and exposed MCP tool names. Actions: write/refresh agent contracts, open agent folder, rotate MCP token.

## Tools menu

**Tools → Internal Index Service** submenu:

- Open Internal Index Service
- Quick Actions: Build Index, Start MCP Server, Stop MCP Server, Open Index Folder

Legacy flat menu entries were removed; full workflows live in the panel tabs.

## Automation

Filter: `InternalIndexService.Editor` (panel, dashboard, governance, settings, retrieval-only invariant, MCP lifecycle, panel status).
