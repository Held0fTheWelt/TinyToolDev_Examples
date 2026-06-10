# IIS LLM Store Bridge

IIS LLM Store Bridge is an optional companion plugin for teams that use both **Internal Index Service** and **LLM Store** in the same Unreal Engine project.

The plugin registers one IIS embedding executor named `llmstore`. IIS can then submit embedding jobs through its normal executor registry while LLM Store remains responsible for provider selection, model routing, runtime modes, local-only rules, fallback policy, governance, and secrets.

## Product Boundary

This plugin is intentionally small. It does not replace or bundle either core product.

| Product | Responsibility |
| --- | --- |
| Internal Index Service (IIS) | Evidence import, indexing, cataloging, retrieval, vector payload storage, context packs, and agent evidence access. |
| IIS LLM Store Bridge | Request/response adaptation between the IIS embedding executor API and the LLM Store embedding route API. |
| LLM Store | Providers, models, routes, policies, runtime modes, governance, secrets, cost rules, and fallbacks. |

Use the bridge when you want IIS embedding jobs to follow LLM Store policy instead of hardcoding provider or model details inside IIS.

## Why Use It

- Route IIS embeddings through stable LLM Store route names such as `iis.embedding.code`.
- Keep IIS independent from provider IDs, model IDs, cloud credentials, and fallback logic.
- Apply LLM Store runtime modes such as mock, local-only, fast, deep, or disabled.
- Use local or deterministic mock embedding workflows for development, tests, and CI-adjacent validation.
- Expose simple Blueprint diagnostics and embedding smoke tests without bypassing LLM Store governance.

## Runtime Flow

```text
IIS embedding job
  -> FIISEmbeddingRequest
  -> IIS executor registry
  -> executor id: llmstore
  -> IIS LLM Store Bridge
  -> FLLMStoreEmbeddingRequest
  -> ILLMStore::ExecuteEmbeddingRoute
  -> FLLMStoreEmbeddingResponse
  -> FIISEmbeddingResponse
  -> IIS vector payload storage
```

Typical IIS embedding task routes:

- `iis.embedding.code`
- `iis.embedding.blueprint`
- `iis.embedding.asset`

LLM Store resolves those routes. The bridge only copies neutral request and response data across the plugin boundary.

## Requirements

- Unreal Engine 5.4.
- Internal Index Service installed and enabled.
- LLM Store installed and enabled.
- LLM Store embedding routes configured or available through defaults.
- A host project or engine plugin setup that contains all three plugins.

The bridge depends only on public interface modules:

- `InternalIndexServiceInterface`
- `LLMStoreInterface`

The plugin module is runtime-only and targets the UE 5.4 plugin line.

## Quick Start

Place all three plugins side by side:

```text
Plugins/
  InternalIndexService/
  LLMStore/
  InternalIndexServiceLLMStoreBridge/
```

Enable them in the `.uproject`:

```json
{
  "Plugins": [
    { "Name": "InternalIndexService", "Enabled": true },
    { "Name": "LLMStore", "Enabled": true },
    { "Name": "InternalIndexServiceLLMStoreBridge", "Enabled": true }
  ]
}
```

Build the project and start the editor. A successful startup should log:

```text
IIS LLM Store Bridge registered embedding executor 'llmstore'.
```

Then verify one embedding path:

1. Configure or confirm the required IIS embedding routes in LLM Store.
2. Import evidence and build embedding jobs in IIS.
3. Execute a small embedding batch.
4. Confirm IIS receives vectors plus provider/model metadata from LLM Store.
5. Disable or block the route in LLM Store and confirm IIS receives a controlled governance failure.

## Blueprint Access

The bridge exposes Blueprint helper nodes through:

```text
UInternalIndexServiceLLMStoreBridgeBlueprintLibrary
```

Node category:

```text
Internal Index Service | LLM Store Bridge
```

Blueprint can check registration, list IIS embedding executors, resolve an IIS embedding route through LLM Store, build an IIS embedding request, and execute one embedding request for diagnostics or smoke testing. Blueprint still does not choose providers, models, or secrets.

See [Blueprint usage](Documentation/BLUEPRINT_USAGE.md) for node-by-node examples.

## What It Does Not Do

The bridge does not:

- index Unreal projects;
- extract Blueprint or C++ evidence;
- store vectors;
- run search or retrieval;
- choose providers or models itself;
- store provider secrets;
- call external providers directly;
- execute chat or completion routes;
- generate AI answers;
- mutate assets, Blueprints, source files, or project settings.

All model routing stays inside LLM Store. All indexing and vector persistence stay inside IIS.

## Documentation

- [Documentation overview](Documentation/README.md)
- [Buyer guide](Documentation/BUYER_GUIDE.md)
- [Quickstart](Documentation/QUICKSTART.md)
- [Blueprint usage](Documentation/BLUEPRINT_USAGE.md)
- [Integration details](Documentation/INTEGRATION.md)
- [Source overview](Documentation/SOURCE_OVERVIEW.md)
- [Troubleshooting](Documentation/TROUBLESHOOTING.md)
- [Release checklist](Documentation/RELEASE_CHECKLIST.md)

## Support

For questions, setup help, and product support, join the Discord server:

https://discord.gg/HycgjVkK4J

## Security and Governance

The bridge never receives or stores provider secrets. Provider credentials remain owned by LLM Store.

The bridge does not bypass LLM Store route governance. If a route is disabled, unavailable, blocked by runtime mode, blocked by local-only policy, or missing a valid provider/model, IIS receives a controlled embedding failure with the LLM Store error code and message.

## Version

Current bridge version: `0.1.0`.

This version targets the IIS and LLM Store public interfaces available in the Unreal Engine 5.4 plugin line.
