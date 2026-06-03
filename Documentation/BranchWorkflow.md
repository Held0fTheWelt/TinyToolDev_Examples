# Branch Workflow

This project is a shared base for Tiny Tool Development examples. The base branch should stay small, neutral, and stable. Every real sample lives on a dedicated branch.

## Branch Roles

`main` is the common foundation:

- Project Settings and neutral metadata.
- Shared documentation and conventions.
- Minimal C++ module.
- No product-specific plugin enabled by default.

`example/<product>/<topic>` branches contain one focused product sample:

- Required plugin configuration.
- Tutorial map or entry Blueprint.
- C++ sample classes when useful.
- Buyer-facing README updates.
- Verification steps.

`integration/<products>/<topic>` branches may combine products, but should only be used when the integration itself is the point of the sample.

## Creating A New Example Branch

Start from `main`:

```bash
git checkout main
git pull
git checkout -b example/<product>/<topic>
```

Then prepare the branch:

1. Enable the required plugin in `TinyToolDev_Examples.uproject`.
2. Add or reference the required plugin under `Plugins/` according to the product license and distribution model.
3. Replace `Documentation/CurrentExample.md` with the branch tutorial.
4. Add sample maps, Blueprints, widgets, data assets, or C++ classes.
5. Link to product documentation from the plugin `.uplugin` metadata when available.
6. Document any setup that cannot be inferred from the project.
7. Run the verification steps listed in `Documentation/ExampleBranchChecklist.md`.

## Naming Guidelines

Prefer branch names that describe the product and the learning goal:

```text
example/llmstore/quickstart
example/llmstore/blueprint-scenario-runner
example/tinytoolkit/editor-utility-widget
integration/llmstore-umg/chat-panel
```

Avoid catch-all branches such as `examples`, `demo`, or `test`. They become hard to explain to buyers and hard to keep current.

## README Strategy

The root `README.md` should stay mostly neutral. On an example branch, add a short "Current Example" section near the top with:

- Product name.
- What the buyer will learn.
- First map or asset to open.
- Link to `Documentation/CurrentExample.md`.

Keep the deep tutorial in `Documentation/CurrentExample.md`. This prevents the root README from becoming too long while still making the branch understandable on GitHub.

## Project Settings Strategy

The base branch uses generic Tiny Tool Development project metadata. Example branches may update:

- Project description.
- Displayed title.
- Default map.
- Enabled plugins.
- Input settings.
- Maps and modes.

Do not change company, support, license, or privacy fields unless the branch genuinely needs a product-specific statement.

## Content Strategy

Use this layout when a branch needs more than one file:

```text
Content/<Product>/<ExampleName>/
Source/TinyToolDev_Examples/Public/<Product>/
Source/TinyToolDev_Examples/Private/<Product>/
Documentation/CurrentExample.md
Examples/<Product>/<ExampleName>/
```

Use `Examples/` for JSON, CSV, prompts, config snippets, test input, or other non-Unreal sample data. Use `Content/` for Unreal assets.

## Cleanup Before Publishing A Branch

Before sharing a branch with buyers:

```bash
git status --short
git diff --check
python3 -m json.tool TinyToolDev_Examples.uproject > /dev/null
```

Also open the project once in Unreal Engine and make sure the documented map or Blueprint entry point works from a clean editor start.
