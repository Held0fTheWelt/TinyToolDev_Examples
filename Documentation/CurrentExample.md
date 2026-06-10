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

- Module name: SmartContentDietIISSimilarityBridge
- Plugin loading phase: PostEngineInit
- Engine version: 5.4.0
- Documentation URL: 

Branch quality note: keep this plugin folder self-contained and verify that the branch opens with TinyToolDev_Examples.uproject.
