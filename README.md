# Tiny Tool Development Examples

<img src="Documentation/Assets/TinyToolDevelopmentIcon.png" alt="Tiny Tool Development icon" width="128">

This repository uses `main` as the neutral Unreal Engine 5.4 example baseline. Product-specific samples live on dedicated branches.

## Current Branch

You are reading the `SmartContentDiet-Examples_v.5.4` branch.

| Branch | Product | Unreal Version | Purpose |
| --- | --- | --- | --- |
| [`LLM-Store-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/LLM-Store-Examples_v.5.4) | LLM Store | Unreal Engine 5.4 | C++ and Blueprint examples for provider-neutral AI routes, mock setup, prompt scenarios, policies, and buyer documentation. |
| [`SmartContentDiet-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDiet-Examples_v.5.4) | SmartContentDiet | Unreal Engine 5.4 | Custom `ISmartSimilarityProvider`, registry registration at third-party tier, and buyer documentation for the open interface. |

Useful branch commands:

```bash
git fetch origin
git checkout main
git checkout SmartContentDiet-Examples_v.5.4
```

## Merge Baseline

The root README is intentionally organized to make regular merges from `main` easier:

- Keep repository-wide information in the shared baseline sections below.
- Keep product-specific tutorial detail in `Documentation/CurrentExample.md` and product-specific docs.
- When merging `main`, prefer the `main` version for shared branch workflow text and keep the `Current Branch` plus `SmartContentDiet Example` sections from this branch.

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

Shared documentation:

- `Documentation/BranchWorkflow.md` explains how examples should be created and maintained.
- `Documentation/ExampleBranchChecklist.md` lists the minimum quality bar for a new branch.
- `Documentation/CurrentExample.md` explains the currently checked-out example branch.
- `Examples/README.md` explains where branch-specific non-asset sample data can live.
- `Plugins/README.md` explains how product plugins should be handled by branches.

## SmartContentDiet Example

This branch is a buyer- and beginner-friendly companion for **SmartContentDiet**, the Unreal Engine plugin for content-health signals, similarity grouping, and governance-aware review workflows.

Use it when you want to see how a game or tool module implements `ISmartSimilarityProvider` and registers with the SmartContentDiet funnel without modifying the core plugin.

## Plugin Requirement

This branch requires the separately distributed `SmartContentDiet` plugin. The plugin package is intentionally not part of this example repository. Install it before opening the project:

- as a project plugin under `Plugins/SmartContentDiet`, or
- as an engine plugin available to Unreal Engine 5.4.

`Plugins/SmartContentDiet` is ignored on purpose so locally installed plugin packages, private builds, binaries, and credentials do not accidentally enter the example repository.

This is the intended validation workflow for maintainers: copy the original distributed plugin into `Plugins/SmartContentDiet`, build or run the examples, and keep the plugin package local.

## Start Here

1. Install SmartContentDiet under `Plugins/SmartContentDiet` (see `Plugins/README.md`).
2. Open `TinyToolDev_Examples.uproject` in Unreal Engine 5.4.
3. Read `Documentation/CurrentExample.md` for the full walkthrough.
4. Open the SmartContentDiet editor UI and confirm the similarity funnel lists the built-in baseline and, after this sample module loads, the `example-prefix` third-party provider.

## Important Files

| File | What to read there |
| --- | --- |
| `Documentation/CurrentExample.md` | Branch tutorial: baseline, custom provider, registration, verification. |
| `Source/TinyToolDev_Examples/Public/ExampleCustomSimilarityProvider.h` | Minimal `ISmartSimilarityProvider` implementation. |
| `Source/TinyToolDev_Examples/Private/TinyToolDev_ExamplesModule.cpp` | Registers the example provider at `ThirdParty` tier on startup. |
| `Source/TinyToolDev_Examples/Private/ExampleProviderTests.cpp` | Automation test for folder-prefix grouping. |
| `Plugins/README.md` | Notes about the intentionally external plugin dependency. |

## Documentation Map

Public SmartContentDiet sources and interface headers live in the TinyToolDevelopment monorepo:

[https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/GovernanceDevelopmentPlugins/SmartContentDiet](https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/GovernanceDevelopmentPlugins/SmartContentDiet)

Content-health evidence contract (for `ISmartContentHealthSource` consumers):

[scd_health_evidence_contract.md](https://github.com/Held0fTheWelt/TinyToolDevelopment/blob/master/docs/architecture/scd_health_evidence_contract.md)

## Verification

```text
Build TinyToolDev_ExamplesEditor Win64 Development -Project=<path>/TinyToolDev_Examples.uproject
Automation RunTests SmartContentDietExamples -NullRHI
```

Expected: `SmartContentDietExamples.CustomProvider.GroupsByPrefix` reports success after the original SmartContentDiet plugin is installed locally.

## Repository Policy

Generated Unreal folders such as `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, `.vs`, and plugin generated output are ignored. Commit source, config, docs, and intentional assets only.

Large `.uasset` and `.umap` files are treated as binary files. Use Git LFS in the remote repository when example branches start carrying larger assets.

The shared project icon lives in `Build/Windows/Application.ico`; the source PNG is kept in `Build/Windows/Application.png` and mirrored for documentation in `Documentation/Assets/TinyToolDevelopmentIcon.png`.

## About Tiny Tool Development

Tiny Tool Development creates focused Unreal Engine tools, plugins, and learning material with practical C++ and Blueprint usage in mind.

- Homepage: https://github.com/Held0fTheWelt/TinyToolDevelopment
- Support: https://discord.gg/HycgjVkK4J
