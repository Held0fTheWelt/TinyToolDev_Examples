# Tiny Tool Development Examples

Neutral Unreal Engine base project for Tiny Tool Development sample content.

This repository is intentionally product-agnostic on the main branch. Concrete examples, tutorials, plugin demos, and buyer-facing walkthroughs live on dedicated Git branches. That keeps every sample focused, easy to compare, and easy to reset.

## What This Branch Contains

- A clean Unreal Engine 5.4 C++ project.
- Tiny Tool Development project metadata in Project Settings.
- Shared documentation for branch-based example work.
- Git hygiene for Unreal-generated files.
- No product-specific plugin dependency in the base branch.

The current branch does not demonstrate a specific Tiny Tool Development product. Checkout an example branch to see real sample content.

## Branch Model

Use `main` as the neutral base. Each example should use its own branch:

```text
main
example/<product>/<topic>
example/<product>/<topic>-blueprint
example/<product>/<topic>-cpp
integration/<product-a>-<product-b>/<topic>
```

Examples:

```text
example/llmstore/quickstart
example/llmstore/blueprint-chat
example/advancedtween/cpp-runtime
integration/llmstore-ui/debug-panel
```

Each branch should be self-contained. A buyer should be able to checkout the branch, open the `.uproject`, read the branch README, and understand what to try first.

## How To Use

1. Clone or open this project.
2. Checkout the example branch you want to inspect.
3. Open `TinyToolDev_Examples.uproject` with Unreal Engine 5.4.
4. Read `Documentation/CurrentExample.md` on that branch.
5. Follow the branch-specific C++ or Blueprint tutorial.

Useful commands:

```bash
git branch --all
git checkout example/<product>/<topic>
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

## About Tiny Tool Development

Tiny Tool Development creates focused Unreal Engine tools, plugins, and learning material with practical C++ and Blueprint usage in mind.

- Homepage: https://github.com/Held0fTheWelt/TinyToolDevelopment
- Support: https://discord.gg/HycgjVkK4J
