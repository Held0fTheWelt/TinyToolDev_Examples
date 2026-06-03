# Current Example: LLM Store

This branch demonstrates **LLM Store** in Unreal Engine 5.4.

LLM Store centralizes AI providers, models, task routes, policies, costs, secrets, and optional agent integrations. This example shows how a buyer can call those routes from C++ and Blueprint without hard-coding provider names, endpoints, model names, or API keys.

## First Run

1. Open `TinyToolDev_Examples.uproject` in Unreal Engine 5.4.
2. Confirm that the bundled `LLMStore` plugin is enabled.
3. Open `Tools -> LLM Store`.
4. Verify that the mock `default` route resolves.
5. Place `LLMStoreExampleActor` in any level and press Play.

The first run uses the local mock provider from `Config/LLMStore.json`, so it does not need a network connection or API key.

## Main Entry Points

- `Documentation/ConsumerGuide.md` is the full buyer tutorial.
- `Documentation/BlueprintRecipes.md` explains the Blueprint-facing sample patterns.
- `Documentation/SampleContent.md` documents routes, prompt templates, and prompt scenarios.
- `Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h` is the simplest placeable C++ actor.
- `Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h` is useful for demo menus.
- `Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h` is useful for chat or NPC-dialogue prototypes.
- `Examples/LLMStore/PromptScenarios.json` contains plain-text scenario prompts.

## Plugin Documentation

The bundled plugin documentation lives in `Plugins/LLMStore/Documentation/`. Start with:

- `Plugins/LLMStore/Documentation/README.md`
- `Plugins/LLMStore/Documentation/QUICKSTART.md`
- `Plugins/LLMStore/Documentation/BUYER_GUIDE.md`
- `Plugins/LLMStore/Documentation/RoutePolicyReference.md`
