# Source Overview

This bridge is intentionally small. Its source exists to make one boundary explicit:

```text
IIS public embedding executor API
  -> IIS LLM Store Bridge adapter
  -> LLM Store public embedding route API
```

## Runtime Module

Source:

```text
Source/InternalIndexServiceLLMStoreBridge/
  Public/InternalIndexServiceLLMStoreBridgeBlueprintLibrary.h
  Public/InternalIndexServiceLLMStoreBridgeModule.h
  Private/InternalIndexServiceLLMStoreBridgeBlueprintLibrary.cpp
  Private/InternalIndexServiceLLMStoreBridgeModule.cpp
```

The module registers one IIS embedding executor named `llmstore` during startup and unregisters it during shutdown.

## Blueprint Library

`UInternalIndexServiceLLMStoreBridgeBlueprintLibrary` exposes the bridge through Blueprint:

- executor readiness checks;
- registered executor id listing;
- route resolution through the bridge;
- IIS embedding request creation;
- simple and full embedding execution.

The Blueprint nodes still use the IIS executor registry, so LLM Store remains the only provider/model governance layer.

## Executor Flow

`FLLMStoreIISEmbeddingRouteExecutor` implements the IIS executor interface:

- `GetExecutorId()` returns the stable executor id `llmstore`.
- `ResolveEmbeddingRoute(...)` asks LLM Store to resolve the IIS task kind.
- `ExecuteEmbeddingRoute(...)` converts the IIS request into an LLM Store embedding request and copies the response back into the IIS response type.

## LLM Store Lookup

The bridge resolves `LLMStoreSubsystem` at call time through the public `ILLMStore` interface. This avoids adding provider-specific behavior to the bridge and keeps the failure mode clear when LLM Store is not installed or not enabled.

## Data Mapping

The bridge copies between IIS and LLM Store request/response structs field by field. This is deliberate:

- IIS and LLM Store remain separately owned plugins.
- API drift becomes visible during compile.
- The bridge cannot accidentally take ownership of indexing, provider routing, secrets, or vector storage.

## Non-Goals in Source

The source code must not add:

- indexing or retrieval logic;
- vector storage;
- provider/model selection;
- provider secrets;
- direct Ollama, LM Studio, llama.cpp, vLLM, or TGI calls;
- chat/completion execution;
- Unreal asset, Blueprint, source, or project mutation.
