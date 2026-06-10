# Current Example: IIS LLM Store Bridge

This branch demonstrates the bridge plugin **IIS LLM Store Bridge** in Unreal Engine 5.4.

## What this plugin does

Interface-only bridge that lets Internal Index Service execute embedding routes through LLM Store without coupling either core plugin.

## First Run

1. Open TinyToolDev_Examples.uproject in Unreal Engine 5.4.
2. Ensure Plugins/InternalIndexServiceLLMStoreBridge is present in this repository branch.
3. Enable the plugin in the project plugin settings if required by your project settings.
4. Build the project editor target for Win64 Development and follow your plugin-specific tests/docs if available.

## Required dependencies

- InternalIndexService
- LLMStore

## Plugin metadata

- Module name: InternalIndexServiceLLMStoreBridge
- Plugin loading phase: PostEngineInit
- Engine version: 5.4.0
- Documentation URL: https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/bridge-plugins/InternalIndexServiceLLMStoreBridge

Branch quality note: keep this plugin folder self-contained and verify that the branch opens with TinyToolDev_Examples.uproject.
