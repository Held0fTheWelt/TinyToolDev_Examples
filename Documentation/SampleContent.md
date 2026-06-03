# Sample Content Catalog

This project includes more than one route so buyers can see how LLM Store scales beyond a single `default` prompt.

All routes in this sample still point to the built-in `mock` provider. That keeps the project safe, deterministic, and offline. When you are ready to use a real provider, keep the route names and change the provider/model configuration in `Tools -> LLM Store`.

## Included Routes

| Route | Workflow | Prompt template | Example use |
| --- | --- | --- | --- |
| `default` | Demo | `example.short.answer` | First smoke test. |
| `asset.explain` | Content | `example.asset.explain` | Explain an Unreal asset to artists/designers. |
| `docs.summarize` | Documentation | `example.docs.summarize` | Summarize plugin or project notes. |
| `npc.dialogue` | Narrative | `example.npc.dialogue` | Draft short placeholder NPC lines. |
| `code.review` | Engineering | `example.code.review` | Review implementation notes or snippets. |
| `qa.testplan` | Quality | `example.qa.testplan` | Turn a feature note into QA checks. |

## Prompt Scenario Data

The JSON file [../Examples/LLMStore/PromptScenarios.json](../Examples/LLMStore/PromptScenarios.json) mirrors the routes above. It is intentionally plain text so it can be:

- read by buyers without opening the editor
- copied into documentation
- used as seed data for a UI widget
- compared against Blueprint scenario nodes
- adapted for studio-specific route catalogs

## Blueprint Scenario Library

The C++ class `ULLMStoreExampleScenarioLibrary` exposes the same scenarios to Blueprint.

Useful nodes:

- `Get LLM Store Example Scenarios`
- `Find LLM Store Example Scenario`
- `Make LLM Request From Scenario`
- `Execute LLM Store Example Scenario`
- `Format LLM Store Example Scenario`

Suggested beginner graph:

1. `Get LLM Store Example Scenarios`
2. Pick one entry from the array
3. `Format LLM Store Example Scenario`
4. Display the formatted text in a widget or `Print String`
5. `Execute LLM Store Example Scenario`
6. Format the response with `Format LLM Response`

For placeable and component-based examples, also see:

- `ALLMStoreScenarioRunnerActor`: runs one scenario by id or all scenarios sequentially.
- `ULLMStoreConversationComponent`: keeps a short local transcript and sends chat-style prompts.
- [BlueprintRecipes.md](BlueprintRecipes.md): step-by-step Blueprint usage recipes.

## Why These Routes Matter

The important lesson is that route names describe work, not vendors.

Good route names:

```text
asset.explain
docs.summarize
npc.dialogue
code.review
qa.testplan
```

Less useful route names:

```text
openai-call
gpt4-request
ollama-button
```

When routes describe work, you can later move `code.review` to a local model, keep `npc.dialogue` on a creative cloud model, and keep `qa.testplan` on a cheap fast model without changing the consuming Blueprint graph.

## Turning This Into Real Project Content

1. Keep the route names that match your workflows.
2. Replace `quickstart_mock_model` with real local or cloud models.
3. Adjust each route policy.
4. Add cost rules for cloud models.
5. Keep secrets out of `Config/LLMStore.json`.
6. Let tools, widgets, and gameplay systems call only the route.

For provider setup, use the separately distributed LLM Store plugin documentation:

https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore
