# Blueprint And C++ Sample Recipes

This file collects the extra sample classes that are useful when a buyer asks, "How would I actually use this in my project?"

The project does not rely on binary Blueprint assets for these samples. Instead, the C++ classes are `Blueprintable`, `BlueprintSpawnableComponent`, or expose Blueprint nodes. That keeps the examples readable in source control and lets users create their own Blueprints from clean base classes.

## Recipe 1: One Actor Smoke Test

Class:

```text
ALLMStoreExampleActor
```

Use it when:

- you want the smallest route execution example
- you want to place one actor in a level and press Play
- you want status and response events split into beginner-friendly pins

Blueprint steps:

1. Place `LLMStoreExampleActor`.
2. Bind `On Example Status` to `Print String`.
3. Bind `On Response` to a Text widget or `Print String`.
4. Press Play.

## Recipe 2: Demo Menu Scenario Runner

Class:

```text
ALLMStoreScenarioRunnerActor
```

Use it when:

- you want a menu of prompt examples
- each button should run a scenario by id
- you want to run all examples sequentially during a presentation

Blueprint steps:

1. Place `LLMStoreScenarioRunnerActor`.
2. Bind `On Scenario Runner Status`.
3. Bind `On Scenario Started`.
4. Bind `On Scenario Finished`.
5. Create buttons for ids from `Get LLM Store Example Scenarios`.
6. On button click, call `Run Scenario By Id`.

Useful scenario ids:

```text
quick-default
asset-explain-crate
docs-summarize-plugin
npc-dialogue-merchant
code-review-async
qa-testplan-save
```

## Recipe 3: Chat Or Dialogue Component

Class:

```text
ULLMStoreConversationComponent
```

Use it when:

- you want a simple gameplay or UMG chat prototype
- you want short local history sent with each request
- you want to demonstrate route-based NPC dialogue

Blueprint steps:

1. Add `LLMStoreConversationComponent` to an actor.
2. Set `Task Kind` to `npc.dialogue` or `default`.
3. Bind `On Conversation Status`.
4. Bind `On Assistant Response`.
5. Call `Send User Message` from a UI button.
6. Display `Get Transcript Text`.

The component keeps transcript history locally. It does not store messages in LLM Store and does not clear the cost ledger or response cache.

## Recipe 4: Pure Node Workflow

Class:

```text
ULLMStoreExampleBlueprintLibrary
```

Use it when:

- you do not want a placed actor
- you want a compact Blueprint graph
- you are building an editor utility, UMG widget, or subsystem

Useful nodes:

- `Get LLM Store`
- `Ensure Example Sample Content`
- `Make Example LLM Request`
- `Resolve Example Route`
- `Execute Example Prompt`
- `Format LLM Response`
- `Format LLM Readiness`

## Recipe 5: Scenario Data Workflow

Class:

```text
ULLMStoreExampleScenarioLibrary
```

Use it when:

- you need source-control-friendly sample prompt data
- you want to build a list of example buttons
- you want to show buyers multiple realistic workflows

Useful nodes:

- `Get LLM Store Example Scenarios`
- `Find LLM Store Example Scenario`
- `Make LLM Request From Scenario`
- `Execute LLM Store Example Scenario`
- `Format LLM Store Example Scenario`

## Why There Are No Required Binary Blueprint Assets

Binary `.uasset` Blueprints are useful in final demos, but source examples are easier for buyers to audit, diff, copy, and learn from. These classes are designed so users can create Blueprints in the editor:

- `BP_LLMStoreScenarioRunner` derived from `LLMStoreScenarioRunnerActor`
- `BP_LLMStoreConversationPawn` with `LLMStoreConversationComponent`
- `WBP_LLMStoreScenarioMenu` using the scenario library nodes

This keeps the sample package lightweight while still giving Blueprint users concrete building blocks.
