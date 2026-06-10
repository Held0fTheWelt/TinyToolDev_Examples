# Blueprint Usage

The IIS LLM Store Bridge is usable from Blueprint through:

```text
UInternalIndexServiceLLMStoreBridgeBlueprintLibrary
```

Node category:

```text
Internal Index Service | LLM Store Bridge
```

## What Blueprint Can Do

Blueprint can:

- check whether the bridge executor is registered;
- list registered IIS embedding executors;
- resolve IIS embedding routes through LLM Store;
- create IIS embedding request structs;
- execute one embedding request through LLM Store governance;
- run a simple one-text embedding call.

Blueprint does not choose providers or models. LLM Store still owns provider routing, runtime modes, governance, secrets, and fallbacks.

## Nodes

### Get IIS LLM Store Bridge Executor Id

Returns:

```text
llmstore
```

Use this for diagnostics and logging.

### Is IIS LLM Store Bridge Registered

Returns `true` when the bridge has registered its IIS embedding executor.

Recommended first node in diagnostic Blueprints:

```text
Event
  -> Is IIS LLM Store Bridge Registered
  -> Branch
```

### Get IIS Embedding Executor Ids

Returns all executor ids currently registered with IIS.

This helps verify whether `llmstore` is present.

### Resolve IIS Embedding Route via LLM Store

Inputs:

```text
TaskKind
```

Outputs:

```text
OutRoute
OutWarnings
ReturnValue
```

Typical task kinds:

```text
iis.embedding.code
iis.embedding.blueprint
iis.embedding.asset
```

The resolved route may expose provider/model ids for diagnostics, but Blueprint callers should not hard-code those ids into IIS workflows.

### Make IIS Embedding Request

Builds an `FIISEmbeddingRequest` struct.

Useful fields:

```text
RouteId
TaskKind
InputText
Metadata
RequestedDimensions
bNormalize
bLocalOnly
```

For most local IIS workflows:

```text
bNormalize = true
bLocalOnly = true
RequestedDimensions = 0
```

### Execute IIS Embedding Route via LLM Store

Executes a complete `FIISEmbeddingRequest`.

Outputs:

```text
FIISEmbeddingResponse
  bSuccess
  RouteId
  ProviderId
  ModelId
  Vector
  Dimensions
  ErrorCode
  ErrorMessage
```

If governance blocks the route, the response contains an error code and message.

### Execute Simple IIS Embedding via LLM Store

Convenience node for one text input.

Inputs:

```text
RouteId
TaskKind
InputText
```

For a default IIS code chunk embedding:

```text
RouteId = iis.embedding.code
TaskKind = iis.embedding.code
InputText = your text
```

## Recommended Blueprint Graphs

### Diagnostic Graph

```text
Event BeginPlay or Editor Utility Button
  -> Is IIS LLM Store Bridge Registered
  -> Get IIS Embedding Executor Ids
  -> Resolve IIS Embedding Route via LLM Store("iis.embedding.code")
  -> Print warnings if ReturnValue is false
```

### Simple Embedding Smoke Test

```text
Editor Utility Button
  -> Execute Simple IIS Embedding via LLM Store
       RouteId = "iis.embedding.code"
       TaskKind = "iis.embedding.code"
       InputText = "IIS bridge Blueprint smoke test"
  -> Branch on Response.bSuccess
  -> Print Dimensions / ErrorCode
```

## Guardrails

Blueprint nodes in this bridge:

- do not index projects;
- do not store vectors;
- do not run search;
- do not call provider APIs directly;
- do not execute chat/completion routes;
- do not mutate Unreal assets, Blueprints, source files, or project settings.

All embedding execution still passes through LLM Store governance.

