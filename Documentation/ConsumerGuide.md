# LLM Store Example Consumer Guide

This guide is written for buyers, technical artists, Blueprint users, tools programmers, and Unreal Engine teams who want to understand how to consume **LLM Store** in a real project.

It focuses on the example project, not the full plugin internals. For complete reference material, use the plugin documentation linked at the end of this file. The same public documentation URL is also stored in `Plugins/LLMStore/LLMStore.uplugin` as `DocsURL`.

## The Short Version

LLM Store lets project code ask for an AI **task route** instead of asking for a specific provider and model.

Your code says:

```text
TaskKind = default
```

LLM Store decides:

```text
Provider -> Model -> Policy -> Secret -> Cost -> Response
```

That separation is the point. It means a Blueprint tool, editor widget, or gameplay system can keep calling the same route while project owners change the actual provider from mock to Ollama, OpenAI, Anthropic, or another backend.

## What This Sample Demonstrates

This project demonstrates the smallest useful integration pattern:

1. Enable the `LLMStore` plugin.
2. Define a provider, model, and route in `Config/LLMStore.json`.
3. Get `ULLMStoreSubsystem` from `GEngine`.
4. Build an `FLLMRequest`.
5. Execute a route.
6. Receive an `FLLMResponse`.
7. Display success, content, model id, token count, cost estimate, or error message.

The example uses the built-in `mock` provider. It requires no API key, no local model server, and no internet connection.

## Project Files To Read First

| File | Reason to read it |
| --- | --- |
| `README.md` | Fast overview and buyer-facing feature map. |
| `Documentation/SampleContent.md` | Route catalog, prompt scenarios, and scenario-library workflow. |
| `Examples/LLMStore/PromptScenarios.json` | Plain-text scenario data that can be copied into UI or documentation. |
| `Config/LLMStore.json` | Shows the provider/model/route setup in committed project config. |
| `Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h` | Shows the Blueprint-facing surface. |
| `Source/TinyToolDev_Examples/Private/LLMStoreExampleActor.cpp` | Shows the tutorial flow in executable C++. |
| `Source/TinyToolDev_Examples/Public/LLMStoreExampleBlueprintLibrary.h` | Shows helper nodes for compact Blueprint graphs. |
| `Source/TinyToolDev_Examples/Private/LLMStoreExampleBlueprintLibrary.cpp` | Shows how requests are normalized and responses are formatted. |
| `Source/TinyToolDev_Examples/Public/LLMStoreExampleScenarioLibrary.h` | Shows source-control-friendly sample content exposed to Blueprint. |
| `Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h` | Shows a placeable demo-menu runner actor. |
| `Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h` | Shows a Blueprint-spawnable chat/dialogue component. |
| `Documentation/BlueprintRecipes.md` | Shows how to use each sample class from Blueprint. |

## Concept Map For Beginners

| Word | Meaning in LLM Store |
| --- | --- |
| Provider | The service or local server, such as `mock`, `ollama`, `openai`, or `anthropic`. |
| Model | A concrete model entry served by one provider. |
| Route | A stable task name used by tools and Blueprints. |
| Policy | Rules that allow or block cloud use, local-only use, provider types, context size, or cost. |
| Secret | API key stored outside committed JSON. |
| Readiness | A status summary that tells you whether providers, models, routes, keys, and policies are usable. |
| Request | `FLLMRequest`, the provider-neutral input sent to the store. |
| Response | `FLLMResponse`, the provider-neutral result returned by the store. |

## Running The Example

1. Open the project in Unreal Engine 5.4.
2. Open `Tools -> LLM Store`.
3. Confirm that the `default` route exists.
4. Open or create any level.
5. Place `LLMStoreExampleActor`.
6. Press Play.

By default the actor:

- ensures that a mock setup exists
- asks the store for readiness
- resolves route `default`
- sends a short prompt
- logs `[mock response]`
- broadcasts Blueprint events

## Blueprint Tutorial: Actor Path

This is the easiest path for a first test.

1. Place `LLMStoreExampleActor` in your level.
2. Select the actor.
3. Keep `Task Kind` set to `default`.
4. Keep `Run On Begin Play` enabled.
5. Keep `Ensure Mock Setup On Begin Play` enabled.
6. In the Level Blueprint, select the actor and add an event for `On Example Status`.
7. Connect `Message` to `Print String`.
8. Add an event for `On Response`.
9. Connect `Content` or `Message` to `Print String`.
10. Press Play.

What you should see:

- a readiness line
- a route line showing provider and model
- a final response event

If you enable `Use Streaming`, the actor calls `Execute Route Streaming`. Providers without native streaming may still send only one final chunk, which is normal.

## Blueprint Tutorial: Function Library Path

Use this path when you want to build your own widget or actor from nodes.

Recommended graph:

1. `Event BeginPlay`
2. `Ensure Example Mock Setup`
3. `Get LLM Store`
4. `Make Example LLM Request`
5. From the store pin, call `Execute Route`
6. Bind the completion delegate
7. On completion, call `Format LLM Response`
8. Display the returned string

Why the helper library exists:

- `Get LLM Store` hides the engine-subsystem lookup.
- `Ensure Example Mock Setup` creates a safe offline demo setup.
- `Make Example LLM Request` fills beginner-friendly defaults.
- `Format LLM Response` shows success, model, tokens, cost, and cache state in one string.

These helpers are educational. In production Blueprints you can call `ULLMStoreSubsystem` directly once your graph is comfortable.

## Blueprint Tutorial: Scenario Catalog Path

The sample also includes a scenario catalog for buyer demos and tutorial menus.

Recommended graph:

1. `Get LLM Store Example Scenarios`
2. Loop over the returned array
3. Use `Display Name` for buttons or list entries
4. Use `Format LLM Store Example Scenario` for hover text or debug output
5. On selection, call `Execute LLM Store Example Scenario`
6. Format the final response with `Format LLM Response`

Included scenario routes:

- `default`
- `asset.explain`
- `docs.summarize`
- `npc.dialogue`
- `code.review`
- `qa.testplan`

Each route maps to a mock model in this sample. In a real project, those same route names can point to different local or cloud models.

## Blueprint Tutorial: Scenario Runner Actor

Use `LLMStoreScenarioRunnerActor` when you want a level actor that powers a demo menu.

Suggested graph:

1. Place `LLMStoreScenarioRunnerActor`
2. Create UI buttons for scenario ids
3. On button click, call `Run Scenario By Id`
4. Bind `On Scenario Started` to update the UI title
5. Bind `On Scenario Finished` to display the response

For a guided demo, call `Run All Scenarios Sequentially`. The actor runs each scenario, waits for the response, then starts the next one.

## Blueprint Tutorial: Conversation Component

Use `LLMStoreConversationComponent` when you want a small chat or NPC dialogue prototype.

Suggested graph:

1. Add `LLMStoreConversationComponent` to an actor
2. Set `Task Kind` to `npc.dialogue`
3. Bind `On Conversation Status`
4. Bind `On Assistant Response`
5. Call `Send User Message` from a text box submit event
6. Display `Get Transcript Text`

The component keeps a local transcript and includes a configurable number of previous messages in each prompt. It is intentionally small so beginners can understand the whole flow.

## C++ Tutorial

The C++ integration has four core steps.

### 1. Include the API

```cpp
#include "LLMStoreSubsystem.h"
#include "Interfaces/LLMStoreInterface.h"
```

Use `LLMStoreInterface` for public request/response types. Use `LLMStore` when you need the subsystem implementation functions.

### 2. Get the subsystem

```cpp
ULLMStoreSubsystem* Store = GEngine->GetEngineSubsystem<ULLMStoreSubsystem>();
if (!Store)
{
	return;
}
```

LLM Store is an engine subsystem, so feature code does not create it manually.

### 3. Build a request

```cpp
FLLMRequest Request;
Request.Operation = ELLMRequestOperation::Chat;
Request.TaskKind = TEXT("default");
Request.SystemPrompt = TEXT("You are concise.");
Request.Prompt = TEXT("Explain this in one sentence.");
Request.bUseCache = true;
Request.MaxBudgetTokens = 512;
```

The route name is the important part. The request does not name OpenAI, Ollama, Anthropic, or a specific URL.

### 4. Execute the route

```cpp
FLLMResponseDelegate OnComplete;
OnComplete.BindUFunction(this, TEXT("HandleResponse"));
Store->ExecuteRoute(Request, OnComplete);
```

The handler receives `FLLMResponse`, which contains:

- `Result.bSuccess`
- `Result.Message`
- `Content`
- `ModelId`
- `Usage.TotalTokens`
- `Usage.EstimatedCost`
- `bFromCache`

## What The Example Actor Teaches

`ALLMStoreExampleActor` is intentionally verbose. It is not meant to be the shortest possible code; it is meant to be readable.

The actor teaches this order:

1. `BeginPlay`
2. optionally call `ApplyStarterSetup(ELLMStarterKind::Mock)`
3. call `GetReadiness`
4. call `ResolveRoute`
5. build `FLLMRequest`
6. call `ExecuteRoute` or `ExecuteRouteStreaming`
7. handle response and stream chunks
8. broadcast Blueprint events

That is the normal mental model for consuming LLM Store.

## What The Scenario Library Teaches

`ULLMStoreExampleScenarioLibrary` is sample content in code form. It avoids binary `.uasset` dependencies and gives Blueprint users ready-made prompts.

It teaches:

1. how to model prompt examples as structs
2. how to keep route names provider-neutral
3. how to turn sample data into `FLLMRequest`
4. how to execute that request through the subsystem
5. how to build demo menus without hard-coded provider details

The matching plain-text catalog is stored in `Examples/LLMStore/PromptScenarios.json`.

For the route and template catalog, see `Documentation/SampleContent.md`.

For copyable Blueprint usage patterns, see `Documentation/BlueprintRecipes.md`.

## Changing The Route

For a real project you should usually create task-specific route names, for example:

```text
docs.summarize
asset.explain
dialogue.draft
quest.review
naming.review
```

Then change the actor or Blueprint `Task Kind` property to that route.

The code does not need to know whether `docs.summarize` uses local Ollama today and OpenAI tomorrow.

## Moving From Mock To Ollama

1. Install and start Ollama.
2. Pull a model, for example:

```text
ollama pull llama3.1
```

3. Open `Tools -> LLM Store`.
4. Use Quick-Start `Local (Ollama)` or create:

```text
Provider Type: ollama
BaseUrl: http://127.0.0.1:11434
Model Name: llama3.1
```

5. Point route `default` or a new route to the Ollama model.
6. Test the route.
7. Run the sample again.

## Moving From Mock To Cloud

1. Open `Tools -> LLM Store`.
2. Use Quick-Start `Create OpenAI` or `Create Anthropic`, or create a cloud provider manually.
3. Store the API key through the UI or a configured secret backend.
4. Confirm `Config/LLMStore.json` does not contain the key.
5. Add or review route policy.
6. Add cost rules if budget visibility matters.
7. Test provider, model, and route.

For shipping-sensitive projects, use local-only policies and build rules before enabling cloud providers broadly.

## Common Beginner Problems

| Symptom | What to check |
| --- | --- |
| `LLM Store subsystem is not available` | Plugin enabled in `.uproject`; module dependencies added. |
| `No enabled route for default` | `Config/LLMStore.json` exists; route enabled; actor setup option enabled. |
| Missing provider or model | Model `ProviderId` matches provider `id`; both enabled. |
| Missing API key | Cloud provider key stored in secret backend or env var. |
| Policy rejection | Route policy allows the provider type and cloud/local mode. |
| Local provider timeout | Ollama/llama.cpp/LM Studio server is running and base URL matches. |
| No stream chunks | Provider may not support native streaming; final chunk behavior is normal. |

## Buyer Evaluation Checklist

Use this sample to answer practical buyer questions:

- Can my team call AI from both C++ and Blueprint?
- Can we keep provider/model choices out of feature code?
- Can we start offline without keys?
- Can we later move a route to local or cloud without rewriting tools?
- Can API keys stay out of source control?
- Can costs and tokens be audited?
- Can policies prevent cloud usage for sensitive or shipping workflows?
- Can other plugins connect through stable interfaces?

The answer this sample demonstrates is yes for the integration surface. You still need your own provider accounts or local model servers for non-mock execution.

## Further Documentation

Public documentation URL from `Plugins/LLMStore/LLMStore.uplugin`:

[https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore](https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore)

Local references:

- [Plugin README](../Plugins/LLMStore/Documentation/README.md)
- [Buyer Guide](../Plugins/LLMStore/Documentation/BUYER_GUIDE.md)
- [Quick Start](../Plugins/LLMStore/Documentation/QUICKSTART.md)
- [User Manual](../Plugins/LLMStore/Documentation/UserManual.md)
- [Integration Guide](../Plugins/LLMStore/Documentation/INTEGRATION.md)
- [Provider Reference](../Plugins/LLMStore/Documentation/ProviderReference.md)
- [Route Policy Reference](../Plugins/LLMStore/Documentation/RoutePolicyReference.md)
- [Secret Backends](../Plugins/LLMStore/Documentation/SECRET_BACKENDS.md)
- [Cost Tracking](../Plugins/LLMStore/Documentation/COST_TRACKING.md)
- [Troubleshooting](../Plugins/LLMStore/Documentation/TROUBLESHOOTING.md)
