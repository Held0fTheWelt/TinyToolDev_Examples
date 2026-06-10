# Integration Details

## Runtime Architecture

The bridge module registers one executor during module startup:

```text
ExecutorId = llmstore
```

The executor implements IIS' neutral embedding interface:

```cpp
class IIISEmbeddingRouteExecutor
{
    virtual bool ResolveEmbeddingRoute(...);
    virtual bool ExecuteEmbeddingRoute(...);
};
```

IIS never includes LLM Store headers and never resolves LLM Store modules directly. The bridge owns that adaptation.

## Dependency Direction

```text
InternalIndexService
  -> no LLM Store dependency

LLMStore
  -> no IIS dependency

InternalIndexServiceLLMStoreBridge
  -> InternalIndexServiceInterface
  -> LLMStoreInterface
```

This allows IIS and LLM Store to remain independently usable products.

## Request Mapping

The bridge maps IIS request data into LLM Store request data:

```text
FIISEmbeddingRequest.RouteId             -> FLLMStoreEmbeddingRequest.RouteId
FIISEmbeddingRequest.TaskKind            -> FLLMStoreEmbeddingRequest.TaskKind
FIISEmbeddingRequest.InputText           -> FLLMStoreEmbeddingRequest.InputText
FIISEmbeddingRequest.Metadata            -> FLLMStoreEmbeddingRequest.Metadata
FIISEmbeddingRequest.RequestedDimensions -> FLLMStoreEmbeddingRequest.RequestedDimensions
FIISEmbeddingRequest.bNormalize          -> FLLMStoreEmbeddingRequest.bNormalize
FIISEmbeddingRequest.bLocalOnly          -> FLLMStoreEmbeddingRequest.bLocalOnly
```

The response is mapped back to IIS:

```text
bSuccess
RouteId
ProviderId
ModelId
Vector
Dimensions
ErrorCode
ErrorMessage
Metadata
```

## Route Resolution

When IIS asks for a task route, the bridge calls:

```cpp
ILLMStore::ResolveEmbeddingRoute(TaskKind, OutRoute)
```

LLM Store owns:

- route enablement;
- provider allow lists;
- runtime modes;
- local-only policy;
- model selection;
- fallback policy.

The bridge only copies the resolved route into IIS' neutral route descriptor.

## Embedding Execution

When IIS executes a job, the bridge calls:

```cpp
ILLMStore::ExecuteEmbeddingRoute(Request, Response)
```

On success, IIS receives the vector and stores it in its own vector payload store.

On failure, IIS receives the LLM Store error code and message. Common failures include:

- missing LLM Store subsystem;
- unknown route;
- disabled route;
- provider unavailable;
- model unsupported;
- governance block.

## Security and Governance

The bridge does not persist secrets, tokens, model credentials, or provider configuration.

The bridge does not call provider SDKs. Provider calls, if any, are performed by LLM Store according to its governance configuration.

## Non-Goals

The bridge does not implement:

- vector databases;
- search;
- indexing;
- MCP tools;
- chat/completion calls;
- AI answer generation;
- asset mutation;
- Blueprint mutation;
- source file mutation.

