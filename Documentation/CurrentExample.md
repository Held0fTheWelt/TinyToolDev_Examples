# Current Example: LLM Store

This branch demonstrates **LLM Store** in Unreal Engine 5.4.

LLM Store centralizes AI providers, models, task routes, policies, costs, secrets, and optional agent integrations. This example shows how a buyer can call those routes from C++ and Blueprint without hard-coding provider names, endpoints, model names, or API keys.

## First Run

1. Open `TinyToolDev_Examples.uproject` in Unreal Engine 5.4.
2. Install the separately distributed `LLMStore` plugin.
3. Confirm that the `LLMStore` plugin is enabled.
4. Open `Tools -> LLM Store`.
5. Verify that the mock `default` route resolves.
6. Place `LLMStoreExampleActor` in any level and press Play.

The first run uses the local mock provider from `Config/LLMStore.json`, so it does not need a network connection or API key.

## Main Entry Points

- `Documentation/ConsumerGuide.md` is the full buyer tutorial.
- `Documentation/BlueprintRecipes.md` explains the Blueprint-facing sample patterns.
- `Documentation/SampleContent.md` documents routes, prompt templates, and prompt scenarios.
- `Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h` is the simplest placeable C++ actor.
- `Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h` is useful for demo menus.
- `Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h` is useful for chat or NPC-dialogue prototypes.
- `Examples/LLMStore/PromptScenarios.json` contains plain-text scenario prompts.

## Plugin Requirement

LLMStore is distributed separately and is intentionally not part of this example repository. Install it as a project or engine plugin before opening the sample.

For sample validation, copy the original distributed plugin into `Plugins/LLMStore`, build or run the project, and leave that local plugin package uncommitted.

Public plugin documentation:

https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore
