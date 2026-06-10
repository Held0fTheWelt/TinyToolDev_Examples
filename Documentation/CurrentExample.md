# Current Example: SmartContentDiet PRS Bridge

This branch demonstrates the bridge plugin **SmartContentDiet PRS Bridge** in Unreal Engine 5.4.

## What this plugin does

Interface-only bridge connecting SmartContentDiet placement review with Project Restructure Service dry-run/apply/rollback and read-only SCD health queries.

## First Run

1. Open TinyToolDev_Examples.uproject in Unreal Engine 5.4.
2. Ensure Plugins/SmartContentDietPRSBridge is present in this repository branch.
3. Enable the plugin in the project plugin settings if required by your project settings.
4. Build the project editor target for Win64 Development and follow your plugin-specific tests/docs if available.

## Required dependencies

- SmartContentDiet
- ProjectRestructureService

## Plugin metadata

- Module name: SmartContentDietPRSBridge
- Plugin loading phase: Default
- Engine version: 5.4.0
- Documentation URL: 

Branch quality note: keep this plugin folder self-contained and verify that the branch opens with TinyToolDev_Examples.uproject.
