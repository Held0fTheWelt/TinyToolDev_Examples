# Current Example: SmartContentDiet

This branch demonstrates **SmartContentDiet** in Unreal Engine 5.4.

SmartContentDiet centralizes content-health signals and similarity grouping behind open interfaces. Buyers can use the built-in baseline with zero custom code, or register their own `ISmartSimilarityProvider` implementations. This sample shows the second path with a minimal teaching provider.

## What You Will Learn

- How the similarity slot and provider funnel work without hard-coding connector names.
- That the **built-in baseline** is always available when the plugin loads.
- How to implement `ISmartSimilarityProvider` in your own game module.
- How to register and unregister a provider through `ISmartContentDietRegistry` at the **ThirdParty** tier.
- Where the public contracts live for health evidence (`ISmartContentHealthSource`).

## First Run

1. Copy the released SmartContentDiet plugin package into `Plugins/SmartContentDiet`. Do not commit that folder.
2. Open `TinyToolDev_Examples.uproject` in Unreal Engine 5.4.
3. Build `TinyToolDev_ExamplesEditor` for Win64 Development.
4. Run automation:

```text
UnrealEditor-Cmd.exe TinyToolDev_Examples.uproject -ExecCmds="Automation RunTests SmartContentDietExamples" -TestExit="Automation Test Queue Empty" -unattended -NullRHI -nopause -nosplash -stdout
```

5. Open the SmartContentDiet editor UI and confirm the similarity funnel lists the built-in baseline. After this sample module loads, you should also see the `example-prefix` third-party provider.

## Plugin Requirement

SmartContentDiet is distributed separately and is intentionally not part of this example repository. Install the released package as a project plugin under `Plugins/SmartContentDiet` or as an engine plugin before opening the sample.

Maintainer validation path:

```text
copy <TinyToolDevelopment>/Git/Saved/SmartContentDiet_Release_1.0.1  ->  Plugins/SmartContentDiet
```

## Main Entry Points

| File | Purpose |
| --- | --- |
| `Source/TinyToolDev_Examples/Public/ExampleCustomSimilarityProvider.h` | Folder-prefix grouping demo implementing `ISmartSimilarityProvider`. |
| `Source/TinyToolDev_Examples/Private/TinyToolDev_ExamplesModule.cpp` | Registers the example provider at `ESmartProviderTier::ThirdParty` on startup. |
| `Source/TinyToolDev_Examples/Private/ExampleProviderTests.cpp` | Automation test `SmartContentDietExamples.CustomProvider.GroupsByPrefix`. |

## How The Example Provider Works

- Provider id: `example-prefix`.
- Supports `ESmartSimilarityMode::Perceptual` only.
- Groups assets that share the same `/Game/...` folder path when at least two assets live in that folder.
- Test assets: two trees under `/Game/Trees/` form one group; a lone rock under `/Game/Rocks/` is excluded.

Replace this logic with real perceptual, structural, or semantic inference in your own connector.

## Contract Surfaces

Similarity (this sample):

- `ISmartSimilarityProvider` — implement and register.
- `ISmartContentDietRegistry` — resolve the SmartContentDiet engine subsystem and register/unregister shared providers.

Content health (read-only, not exercised in this branch):

- `ISmartContentHealthSource` — live snapshot queries.
- Evidence manifest export — see the monorepo contract doc.

Public plugin tree:

https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/GovernanceDevelopmentPlugins/SmartContentDiet

Content-health evidence contract:

https://github.com/Held0fTheWelt/TinyToolDevelopment/blob/master/docs/architecture/scd_health_evidence_contract.md

## Verification

```text
D:/Engines/UE_5.4/Engine/Build/BatchFiles/Build.bat TinyToolDev_ExamplesEditor Win64 Development -Project=D:/TinyToolDevelopment/TinyToolDev_Examples/TinyToolDev_Examples.uproject -WaitMutex
```

```text
UnrealEditor-Cmd.exe TinyToolDev_Examples.uproject -ExecCmds="Automation RunTests SmartContentDietExamples" -TestExit="Automation Test Queue Empty" -unattended -NullRHI -nopause -nosplash -stdout
```

Expected: `SmartContentDietExamples.CustomProvider.GroupsByPrefix` → `Result={Success}`.
