# LLM Store Examples for Unreal Engine 5.4

<img src="Documentation/Assets/TinyToolDevelopmentIcon.png" alt="Tiny Tool Development icon" width="128">

This branch is `LLM-Store-Examples_v.5.4` of the shared `TinyToolDev_Examples` project.

This example project is a buyer- and beginner-friendly companion for **LLM Store**, the Unreal Engine plugin that centralizes AI providers, models, task routes, policies, costs, secrets, and optional agent integrations.

Use this project when you want to see how a real Unreal module calls LLM Store from **C++** and **Blueprint** without hard-coding provider names, model names, endpoints, or API keys.

## Why This Sample Exists

LLM Store is an infrastructure plugin. That means the most important idea is not a single widget or one prompt button, but a clean project pattern:

1. Your game, tool, or editor utility asks for a stable task route, for example `default`, `asset.explain`, or `docs.summarize`.
2. LLM Store resolves that route to the configured provider and model.
3. LLM Store applies readiness, policy, secret, cache, usage, and cost rules.
4. Your code receives a provider-neutral `FLLMResponse`.

This sample keeps that flow visible and small.

## Included Example Setup

The project ships with an offline mock setup in [Config/LLMStore.json](Config/LLMStore.json). It includes one smoke-test route plus several realistic sample routes:

| Item | Value | Purpose |
| --- | --- | --- |
| Provider | `quickstart_mock` | Built-in mock provider; no network and no key. |
| Model | `quickstart_mock_model` | Mock model used by the demo route. |
| Routes | `default`, `asset.explain`, `docs.summarize`, `npc.dialogue`, `code.review`, `qa.testplan` | Stable task routes that C++ and Blueprint can call. |
| Policy | local/mock only | Safe first run for demos, tutorials, tests, and CI. |

The mock provider returns `[mock response]` through the same route execution path that a real provider would use. This makes the tutorial safe to run before you configure Ollama, OpenAI, Anthropic, or another provider.

For the full sample catalog, read [Documentation/SampleContent.md](Documentation/SampleContent.md) and [Examples/LLMStore/PromptScenarios.json](Examples/LLMStore/PromptScenarios.json).

## Start Here

1. Open [TinyToolDev_Examples.uproject](TinyToolDev_Examples.uproject) in Unreal Engine 5.4.
2. Confirm the `LLMStore` plugin is enabled in the project.
3. Open `Tools -> LLM Store`.
4. Check the `Status` or `Setup` tab. The sample `default` route should resolve to the mock provider.
5. Place `LLMStoreExampleActor` in any level.
6. Press Play.
7. Watch the output log or bind the actor events in Blueprint.

The actor logs readiness, route resolution, and the final response.

## Important Files

| File | What to read there |
| --- | --- |
| [Documentation/ConsumerGuide.md](Documentation/ConsumerGuide.md) | Full consumer tutorial, buyer notes, Blueprint walkthrough, C++ walkthrough, and links to plugin docs. |
| [Documentation/SampleContent.md](Documentation/SampleContent.md) | Catalog of routes, prompt templates, and scenario workflow ideas. |
| [Documentation/BlueprintRecipes.md](Documentation/BlueprintRecipes.md) | Extra C++/Blueprint recipes: actor smoke test, scenario runner, conversation component, pure node workflow. |
| [Examples/LLMStore/PromptScenarios.json](Examples/LLMStore/PromptScenarios.json) | Plain-text sample prompts that match the Blueprint scenario library. |
| [Config/LLMStore.json](Config/LLMStore.json) | Offline provider/model/route setup used by this sample. |
| [Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h](Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h) | Blueprint-facing actor, properties, events, and tutorial comments. |
| [Source/TinyToolDev_Examples/Private/LLMStoreExampleActor.cpp](Source/TinyToolDev_Examples/Private/LLMStoreExampleActor.cpp) | The complete route execution flow from BeginPlay to response event. |
| [Source/TinyToolDev_Examples/Public/LLMStoreExampleBlueprintLibrary.h](Source/TinyToolDev_Examples/Public/LLMStoreExampleBlueprintLibrary.h) | Compact Blueprint helper nodes for beginner-friendly graphs. |
| [Source/TinyToolDev_Examples/Private/LLMStoreExampleBlueprintLibrary.cpp](Source/TinyToolDev_Examples/Private/LLMStoreExampleBlueprintLibrary.cpp) | Request construction, route resolving, fallback messages, and formatting helpers. |
| [Source/TinyToolDev_Examples/Public/LLMStoreExampleScenarioLibrary.h](Source/TinyToolDev_Examples/Public/LLMStoreExampleScenarioLibrary.h) | Blueprint-accessible prompt scenario catalog. |
| [Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h](Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h) | Placeable actor for demo menus and sequential scenario runs. |
| [Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h](Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h) | Blueprint-spawnable component for simple chat/dialogue prototypes. |

## Blueprint Tutorial

There are two beginner paths.

### Option A: Actor Workflow

1. Place `LLMStoreExampleActor` in a level.
2. Keep `Task Kind` as `default`.
3. Keep `Ensure Mock Setup On Begin Play` enabled for a safe first run.
4. Bind `On Example Status` to `Print String` if you want to see each tutorial step.
5. Bind `On Response` to a Text widget or `Print String`.
6. Press Play.

Useful actor settings:

| Setting | Meaning |
| --- | --- |
| `Run On Begin Play` | Sends the demo prompt automatically. |
| `Ensure Mock Setup On Begin Play` | Creates or validates the mock `default` route. |
| `Use Streaming` | Calls `Execute Route Streaming`; mock providers may still emit one final chunk. |
| `Use Cache` | Enables response cache behavior in the request. |
| `Task Kind` | The route name sent to LLM Store. |
| `Prompt` | User prompt sent through the route. |
| `System Prompt` | Instruction sent with the request. |
| `Max Budget Tokens` | Optional request budget used by LLM Store policies and estimates. |

### Option B: Function Library Workflow

Build this Blueprint graph:

1. `Event BeginPlay`
2. `Ensure Example Mock Setup`
3. `Get LLM Store`
4. `Make Example LLM Request`
5. Drag from the store return value and call `Execute Route`
6. Bind the completion delegate
7. In the completion event, call `Format LLM Response`
8. Display the formatted string

This path exposes the underlying subsystem call more directly. It is a good choice when you want to integrate LLM Store into your own UI widget, editor utility, or gameplay actor.

### Option C: Scenario Catalog Workflow

Use this when you want demo buttons or a tutorial menu.

1. `Get LLM Store Example Scenarios`
2. Show each scenario's `Display Name`
3. On button click, call `Execute LLM Store Example Scenario`
4. Format the result with `Format LLM Response`
5. Display the output

The scenario catalog includes content, documentation, narrative, engineering, and QA examples. All use mock routes by default.

### Option D: Scenario Runner Actor

Use this for a demo map or buyer presentation.

1. Place `LLMStoreScenarioRunnerActor`
2. Bind `On Scenario Runner Status`
3. Bind `On Scenario Started`
4. Bind `On Scenario Finished`
5. Call `Run Scenario By Id` from buttons
6. Or call `Run All Scenarios Sequentially`

Useful ids include `quick-default`, `asset-explain-crate`, `docs-summarize-plugin`, `npc-dialogue-merchant`, `code-review-async`, and `qa-testplan-save`.

### Option E: Conversation Component

Use this for a small gameplay or UMG chat prototype.

1. Add `LLMStoreConversationComponent` to an actor
2. Set `Task Kind` to `npc.dialogue`
3. Bind `On Conversation Status`
4. Bind `On Assistant Response`
5. Call `Send User Message` from a UI button
6. Display `Get Transcript Text`

## C++ Tutorial

Minimal request:

```cpp
#include "LLMStoreSubsystem.h"
#include "Engine/Engine.h"
#include "Interfaces/LLMStoreInterface.h"

void RunLLMStoreRequest(UObject* CallbackObject)
{
	ULLMStoreSubsystem* Store = GEngine->GetEngineSubsystem<ULLMStoreSubsystem>();
	if (!Store)
	{
		return;
	}

	FLLMRequest Request;
	Request.Operation = ELLMRequestOperation::Chat;
	Request.TaskKind = TEXT("default");
	Request.SystemPrompt = TEXT("You are a concise Unreal Engine assistant.");
	Request.Prompt = TEXT("Explain what LLM Store is doing here in one sentence.");
	Request.bUseCache = true;
	Request.MaxBudgetTokens = 512;

	FLLMResponseDelegate OnComplete;
	OnComplete.BindUFunction(CallbackObject, TEXT("OnLLMStoreResponse"));
	Store->ExecuteRoute(Request, OnComplete);
}
```

The full version is in `ALLMStoreExampleActor`. It demonstrates:

- checking that the subsystem exists
- applying the mock starter setup
- reading readiness
- resolving `default` before execution
- building `FLLMRequest`
- calling normal or streaming execution
- forwarding `FLLMResponse` data to Blueprint events

`ULLMStoreExampleScenarioLibrary` adds reusable sample prompts for route-driven UI demos. It demonstrates:

- returning tutorial scenarios as Blueprint structs
- finding a scenario by id
- turning a scenario into `FLLMRequest`
- executing a scenario through the same subsystem API

`ALLMStoreScenarioRunnerActor` demonstrates a placeable C++ actor that can power a Blueprint scenario menu. `ULLMStoreConversationComponent` demonstrates a Blueprint-spawnable component with short local transcript history.

## Moving From Mock to a Real Provider

The sample starts with `mock` because it is deterministic and safe. To use a real provider:

1. Open `Tools -> LLM Store`.
2. In `Setup`, use Quick-Start for Ollama, OpenAI, or Anthropic, or create provider/model entries manually.
3. In `Routes`, point `default` or your own route to the new model.
4. For cloud providers, set the API key in the LLM Store UI or a secret backend. Do not commit keys to JSON.
5. Test provider, model, and route.
6. Keep your C++ and Blueprint code unchanged if the route name stays the same.

That last point is the main consumer benefit: tools call `TaskKind`, while project owners can change providers centrally.

## Buyer Notes

LLM Store is useful when your project has more than one AI-powered feature or when you need governance around provider access.

It helps with:

- local-first development and privacy-sensitive workflows
- cloud providers for higher quality or larger context when allowed
- route-based integration for tools and Blueprints
- cost and token visibility
- secret storage outside committed config
- route policies and build rules
- optional agent integrations
- a single C++/Blueprint surface for AI calls

It does not ship language models, cloud credits, provider accounts, or a full project-specific RAG/indexing product. It provides the central store and extension points that those systems can use.

## Documentation Map

The plugin's public documentation URL is stored in [Plugins/LLMStore/LLMStore.uplugin](Plugins/LLMStore/LLMStore.uplugin) as `DocsURL`:

[https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore](https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore)

Local documentation included with this sample plugin copy:

- [Plugin README](Plugins/LLMStore/Documentation/README.md): feature overview and documentation index.
- [Buyer Guide](Plugins/LLMStore/Documentation/BUYER_GUIDE.md): buyer-friendly value proposition and workflows.
- [Quick Start](Plugins/LLMStore/Documentation/QUICKSTART.md): first provider/model/route setup.
- [User Manual](Plugins/LLMStore/Documentation/UserManual.md): full editor UI workflow.
- [Integration Guide](Plugins/LLMStore/Documentation/INTEGRATION.md): C++ and Blueprint integration patterns.
- [Provider Reference](Plugins/LLMStore/Documentation/ProviderReference.md): supported provider types and base URLs.
- [Route Policy Reference](Plugins/LLMStore/Documentation/RoutePolicyReference.md): policies, fallbacks, and build rules.
- [Secret Backends](Plugins/LLMStore/Documentation/SECRET_BACKENDS.md): encrypted file, OS keychains, 1Password, Azure Key Vault, and env vars.
- [Cost Tracking](Plugins/LLMStore/Documentation/COST_TRACKING.md): ledger, cost rules, reports, and CSV export.
- [Troubleshooting](Plugins/LLMStore/Documentation/TROUBLESHOOTING.md): common setup and runtime fixes.

## Verification

This sample has been verified with:

```text
D:/Engines/UE_5.4/Engine/Build/BatchFiles/Build.bat TinyToolDev_ExamplesEditor Win64 Development -Project=D:/TinyToolDevelopment/TinyToolDev_Examples/TinyToolDev_Examples.uproject -WaitMutex
```

Expected result: UnrealHeaderTool runs, the sample module compiles, and `UnrealEditor-TinyToolDev_Examples.dll` links successfully.
