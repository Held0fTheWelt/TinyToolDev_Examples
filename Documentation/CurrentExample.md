# Current Example: SmartContentDiet IIS Similarity Bridge

This branch demonstrates the bridge plugin **SmartContentDiet IIS Similarity Bridge** in Unreal Engine 5.4.

## What this plugin does

Interface-only bridge that lets SmartContentDiet run semantic similarity through Internal Index Service without coupling either core plugin.

## First Run

1. Open TinyToolDev_Examples.uproject in Unreal Engine 5.4.
2. Ensure Plugins/SmartContentDietIISSimilarityBridge is present in this repository branch.
3. Enable the plugin in the project plugin settings if required by your project settings.
4. Build the project editor target for Win64 Development and follow your plugin-specific tests/docs if available.

## Required dependencies

- InternalIndexService
- SmartContentDiet

## Plugin metadata

- Module name: $(@{FileVersion=3; Version=2; VersionName=1.0.1; FriendlyName=SmartContentDiet IIS Similarity Bridge; Description=Interface-only bridge that lets SmartContentDiet run semantic similarity through Internal Index Service without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; EngineVersion=5.4.0; CanContainContent=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.Modules[0].Name)
- Plugin loading phase: $(@{FileVersion=3; Version=2; VersionName=1.0.1; FriendlyName=SmartContentDiet IIS Similarity Bridge; Description=Interface-only bridge that lets SmartContentDiet run semantic similarity through Internal Index Service without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; EngineVersion=5.4.0; CanContainContent=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.Modules[0].LoadingPhase)
- Engine version: $(@{FileVersion=3; Version=2; VersionName=1.0.1; FriendlyName=SmartContentDiet IIS Similarity Bridge; Description=Interface-only bridge that lets SmartContentDiet run semantic similarity through Internal Index Service without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; EngineVersion=5.4.0; CanContainContent=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.EngineVersion)
- Documentation URL: $(@{FileVersion=3; Version=2; VersionName=1.0.1; FriendlyName=SmartContentDiet IIS Similarity Bridge; Description=Interface-only bridge that lets SmartContentDiet run semantic similarity through Internal Index Service without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; EngineVersion=5.4.0; CanContainContent=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.DocsURL)

Branch quality note: keep this plugin folder self-contained and verify that the branch opens with TinyToolDev_Examples.uproject.
