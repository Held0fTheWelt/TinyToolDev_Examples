# Tiny Tool Development Examples

<img src="Documentation/Assets/TinyToolDevelopmentIcon.png" alt="Tiny Tool Development icon" width="128">

This repository uses `main` as the neutral Unreal Engine 5.4 example baseline. Product-specific samples live on dedicated branches.

## Current Branch

You are reading the `LLM-Store-Examples_v.5.4` branch.

| Branch | Product | Unreal Version | Purpose |
| --- | --- | --- | --- |
| [`LLM-Store-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/LLM-Store-Examples_v.5.4) | LLM Store | Unreal Engine 5.4 | C++ and Blueprint examples for provider-neutral AI routes, mock setup, prompt scenarios, policies, and buyer documentation. |
| [`SmartContentDiet-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDiet-Examples_v.5.4) | SmartContentDiet | Unreal Engine 5.4 | Custom `ISmartSimilarityProvider`, registry registration at third-party tier, and buyer documentation for the open interface. |

Useful branch commands:

```bash
git fetch origin
git checkout main
git checkout LLM-Store-Examples_v.5.4
git checkout SmartContentDiet-Examples_v.5.4
```

## Merge Baseline

The root README is intentionally organized to make regular merges from `main` easier:

- Keep repository-wide information in the shared baseline sections below.
- Keep product-specific tutorial detail in `Documentation/CurrentExample.md`, `Documentation/ConsumerGuide.md`, and product-specific docs.
- When merging `main`, prefer the `main` version for shared branch workflow text and keep the `Current Branch` plus `LLM Store Example` sections from this branch.

This keeps the branch README useful without forcing every product tutorial to conflict with the neutral baseline README.

## Shared Baseline

The shared project baseline provides:

- A clean Unreal Engine 5.4 C++ project.
- Tiny Tool Development project metadata in Project Settings.
- Shared documentation for branch-based example work.
- Git hygiene for Unreal-generated files.
- Tiny Tool Development branding assets for project and packaged Windows builds.

Concrete examples, tutorials, plugin demos, and buyer-facing walkthroughs live on dedicated Git branches. Each branch should be self-contained: a buyer should be able to checkout the branch, open the `.uproject`, read the branch README, and understand what to try first.

Common branch naming:

```text
main
LLM-Store-Examples_v.5.4
SmartContentDiet-Examples_v.5.4
example/<product>/<topic>
example/<product>/<topic>-blueprint
example/<product>/<topic>-cpp
integration/<product-a>-<product-b>/<topic>
```

Examples:

```text
LLM-Store-Examples_v.5.4
SmartContentDiet-Examples_v.5.4
example/llmstore/quickstart
example/llmstore/blueprint-chat
example/advancedtween/cpp-runtime
integration/llmstore-ui/debug-panel
```

Shared documentation:

- `Documentation/BranchWorkflow.md` explains how examples should be created and maintained.
- `Documentation/ExampleBranchChecklist.md` lists the minimum quality bar for a new branch.
- `Documentation/CurrentExample.md` explains the currently checked-out example branch.
- `Examples/README.md` explains where branch-specific non-asset sample data can live.
- `Plugins/README.md` explains how product plugins should be handled by branches.

## Branch Quality Bar

Every example branch should document:

- What product or feature it demonstrates.
- Which plugin version or marketplace package is required.
- Where the product documentation is linked in the related `.uplugin`.
- Which map, Blueprint, C++ class, or widget is the entry point.
- Whether network access, API keys, local files, credentials, or third-party services are needed.
- How to verify the example in less than five minutes.

## LLM Store Example

This branch is a buyer- and beginner-friendly companion for **LLM Store**, the Unreal Engine plugin that centralizes AI providers, models, task routes, policies, costs, secrets, and optional agent integrations.

Use it when you want to see how a real Unreal module calls LLM Store from **C++** and **Blueprint** without hard-coding provider names, model names, endpoints, or API keys.

## Plugin Requirement

This branch requires the separately distributed `LLMStore` plugin. The plugin package is intentionally not part of this example repository. Install it before opening the project:

- as a project plugin under `Plugins/LLMStore`, or
- as an engine plugin available to Unreal Engine 5.4.

`Plugins/LLMStore` is ignored on purpose so locally installed plugin packages, private builds, binaries, and credentials do not accidentally enter the example repository.

This is the intended validation workflow for maintainers: copy the original distributed plugin into `Plugins/LLMStore`, build or run the examples, and keep the plugin package local.

The project ships with an offline mock setup in `Config/LLMStore.json`. It includes these routes:

| Route | Purpose |
| --- | --- |
| `default` | Smoke test route for the first run. |
| `asset.explain` | Content and asset explanation workflow. |
| `docs.summarize` | Documentation summary workflow. |
| `npc.dialogue` | Narrative and dialogue workflow. |
| `code.review` | Engineering review workflow. |
| `qa.testplan` | QA checklist workflow. |

The mock provider returns through the same route execution path that a real provider would use, so the first run needs no network access or API key.

## Start Here

1. Open `TinyToolDev_Examples.uproject` in Unreal Engine 5.4.
2. Confirm the `LLMStore` plugin is installed and enabled.
3. Open `Tools -> LLM Store`.
4. Check that the sample `default` route resolves to the mock provider.
5. Place `LLMStoreExampleActor` in any level.
6. Press Play.
7. Watch the output log or bind the actor events in Blueprint.

## Important Files

| File | What to read there |
| --- | --- |
| `Documentation/CurrentExample.md` | Short branch-specific orientation and plugin documentation links. |
| `Documentation/ConsumerGuide.md` | Full buyer tutorial with Blueprint and C++ walkthroughs. |
| `Documentation/BlueprintRecipes.md` | Actor, scenario runner, conversation component, and pure-node Blueprint recipes. |
| `Documentation/SampleContent.md` | Route, prompt template, and scenario catalog. |
| `Examples/LLMStore/PromptScenarios.json` | Source-control-friendly prompt scenarios. |
| `Config/LLMStore.json` | Offline mock provider, model, route, and policy setup. |
| `Source/TinyToolDev_Examples/Public/LLMStoreExampleActor.h` | Placeable actor for a first C++/Blueprint smoke test. |
| `Source/TinyToolDev_Examples/Public/LLMStoreScenarioRunnerActor.h` | Placeable actor for demo menus and sequential scenario runs. |
| `Source/TinyToolDev_Examples/Public/LLMStoreConversationComponent.h` | Blueprint-spawnable component for chat or NPC-dialogue prototypes. |
| `Plugins/README.md` | Notes about the intentionally external plugin dependency. |

## Documentation Map

Use the public LLM Store documentation for plugin setup and API reference:

[https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore](https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore)

When the plugin is installed locally, the same URL is also available through its `.uplugin` `DocsURL` metadata.

## Verification

This branch has been verified with:

```text
D:/Engines/UE_5.4/Engine/Build/BatchFiles/Build.bat TinyToolDev_ExamplesEditor Win64 Development -Project=D:/TinyToolDevelopment/TinyToolDev_Examples/TinyToolDev_Examples.uproject -WaitMutex -NoLiveCoding
```

Expected result after the original LLMStore plugin is installed locally: UnrealHeaderTool runs, the sample module compiles, and `UnrealEditor-TinyToolDev_Examples.dll` links successfully.

## Repository Policy

Generated Unreal folders such as `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, `.vs`, and plugin generated output are ignored. Commit source, config, docs, and intentional assets only.

Large `.uasset` and `.umap` files are treated as binary files. Use Git LFS in the remote repository when example branches start carrying larger assets.

The shared project icon lives in `Build/Windows/Application.ico`; the source PNG is kept in `Build/Windows/Application.png` and mirrored for documentation in `Documentation/Assets/TinyToolDevelopmentIcon.png`.

## About Tiny Tool Development

Tiny Tool Development creates focused Unreal Engine tools, plugins, and learning material with practical C++ and Blueprint usage in mind.

- Homepage: https://github.com/Held0fTheWelt/TinyToolDevelopment
- Support: https://discord.gg/HycgjVkK4J
