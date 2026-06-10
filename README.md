# Tiny Tool Development Examples

<img src="Documentation/Assets/TinyToolDevelopmentIcon.png" alt="Tiny Tool Development icon" width="128">

This repository is a **multi-branch Unreal example hub** for Tiny Tool Development.

`main` stays a neutral UE 5.4 base project.

Every product or integration story is represented by its own branch so each buyer gets one clean, focused entry point, one plugin setup, one verification path, and one README that explains what to run.

## What is inside `main`

- A clean Unreal Engine 5.4 C++ project.
- Shared project metadata for Tiny Tool Development.
- Branch workflow documentation and templates.
- No product-specific plugin dependencies.
- Source control hygiene for Unreal-generated folders (`Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `.vs`).

## Branch strategy

This repo follows a branch-per-example model:

- `main` contains only shared project infrastructure.
- Every example, bridge, or integration has its own branch.
- The active branch should answer:
  - What product is demonstrated?
  - Which plugins are required?
  - What is the first practical step?
  - What is the quick verification command/test?

## Available Example Branches

| Branch | Focus | Product / Integration | Unreal Version | Notes |
| --- | --- | --- | --- | --- |
| [`LLM-Store-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/LLM-Store-Examples_v.5.4) | Example | LLM Store | 5.4 | End-to-end LLM workflow sample with example actors, prompts, and documentation. |
| [`SmartContentDiet-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDiet-Examples_v.5.4) | Example | SmartContentDiet | 5.4 | Open provider interface and custom similarity example provider. |
| [`InternalIndexService-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/InternalIndexService-Examples_v.5.4) | Example | Internal Index Service | 5.4 | Baseline Internal Index Service usage and extension surface demo. |
| [`InternalIndexServiceLLMStoreBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/InternalIndexServiceLLMStoreBridge-Examples_v.5.4) | Bridge | Internal Index Service ? LLM Store | 5.4 | Bridge plugin dedicated branch for embedding route handoff. |
| [`InternalIndexServiceUMCPBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/InternalIndexServiceUMCPBridge-Examples_v.5.4) | Bridge | Internal Index Service ? Unified MCP | 5.4 | Bridge plugin dedicated branch for MCP tool publishing. |
| [`SmartContentDietIISSimilarityBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDietIISSimilarityBridge-Examples_v.5.4) | Bridge | SmartContentDiet ? Internal Index Service | 5.4 | Bridge plugin dedicated branch for IIS similarity + health evidence import. |
| [`SmartContentDietPRSBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDietPRSBridge-Examples_v.5.4) | Bridge | SmartContentDiet ? Project Restructure Service | 5.4 | Bridge plugin dedicated branch for placement-review and dry-run/apply/rollback support. |
| [`SmartContentDietUIIMigrationBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/SmartContentDietUIIMigrationBridge-Examples_v.5.4) | Bridge | SmartContentDiet ? UII | 5.4 | Bridge plugin dedicated branch for migration-advice surfaces. |
| [`UIIInternalIndexServiceBridge-Examples_v.5.4`](https://github.com/Held0fTheWelt/TinyToolDev_Examples/tree/UIIInternalIndexServiceBridge-Examples_v.5.4) | Bridge | Unreal Integration Intelligence ? Internal Index Service | 5.4 | Bridge plugin branch for handoff/import evidence workflows. |

## How to inspect a branch

```bash
git fetch origin
git checkout <branch-name>
```

Then open `TinyToolDev_Examples.uproject` and read `Documentation/CurrentExample.md` on that branch.

## How To Use

1. Clone or open this project.
2. Checkout the example branch you want to inspect.
3. Open `TinyToolDev_Examples.uproject` with Unreal Engine 5.4.
4. Read `Documentation/CurrentExample.md` on that branch.
5. Follow the branch-specific C++ or Blueprint tutorial.

Useful commands:

```bash
git branch --all
git checkout main
```

## Documentation

- `Documentation/BranchWorkflow.md` explains how examples should be created and maintained.
- `Documentation/ExampleBranchChecklist.md` lists the minimum quality bar for a new branch.
- `Documentation/CurrentExample.md` is a placeholder on `main` and should be replaced on every example branch.
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

## Repository Policy

Generated Unreal folders such as `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, and `.vs` are ignored. Commit source, config, docs, and intentional assets only.

Large `.uasset` and `.umap` files are treated as binary files. Use Git LFS in the remote repository when example branches start carrying larger assets.

The shared project icon lives in `Build/Windows/Application.ico`; the source PNG is kept in `Build/Windows/Application.png` and mirrored for documentation in `Documentation/Assets/TinyToolDevelopmentIcon.png`.

## About Tiny Tool Development

Tiny Tool Development creates focused Unreal Engine tools, plugins, and learning material with practical C++ and Blueprint usage in mind.

- Homepage: https://github.com/Held0fTheWelt/TinyToolDev_Examples
- Support: https://discord.gg/HycgjVkK4J
