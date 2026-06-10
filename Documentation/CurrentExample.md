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

- Module name: $(@{FileVersion=3; Version=1; VersionName=1.0.0; FriendlyName=IIS LLM Store Bridge; Description=Interface-only bridge that lets Internal Index Service execute embedding routes through LLM Store without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; CreatedByURL=https://www.linkedin.com/company/tiny-tool-development; DocsURL=https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/bridge-plugins/InternalIndexServiceLLMStoreBridge; SupportURL=https://www.linkedin.com/company/tiny-tool-development; EngineVersion=5.4.0; CanContainContent=False; IsBetaVersion=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.Modules[0].Name)
- Plugin loading phase: $(@{FileVersion=3; Version=1; VersionName=1.0.0; FriendlyName=IIS LLM Store Bridge; Description=Interface-only bridge that lets Internal Index Service execute embedding routes through LLM Store without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; CreatedByURL=https://www.linkedin.com/company/tiny-tool-development; DocsURL=https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/bridge-plugins/InternalIndexServiceLLMStoreBridge; SupportURL=https://www.linkedin.com/company/tiny-tool-development; EngineVersion=5.4.0; CanContainContent=False; IsBetaVersion=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.Modules[0].LoadingPhase)
- Engine version: $(@{FileVersion=3; Version=1; VersionName=1.0.0; FriendlyName=IIS LLM Store Bridge; Description=Interface-only bridge that lets Internal Index Service execute embedding routes through LLM Store without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; CreatedByURL=https://www.linkedin.com/company/tiny-tool-development; DocsURL=https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/bridge-plugins/InternalIndexServiceLLMStoreBridge; SupportURL=https://www.linkedin.com/company/tiny-tool-development; EngineVersion=5.4.0; CanContainContent=False; IsBetaVersion=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.EngineVersion)
- Documentation URL: $(@{FileVersion=3; Version=1; VersionName=1.0.0; FriendlyName=IIS LLM Store Bridge; Description=Interface-only bridge that lets Internal Index Service execute embedding routes through LLM Store without coupling either core plugin.; Category=AI Plugin Bridges; CreatedBy=Tiny Tool Development; CreatedByURL=https://www.linkedin.com/company/tiny-tool-development; DocsURL=https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/bridge-plugins/InternalIndexServiceLLMStoreBridge; SupportURL=https://www.linkedin.com/company/tiny-tool-development; EngineVersion=5.4.0; CanContainContent=False; IsBetaVersion=False; Installed=False; Modules=System.Object[]; Plugins=System.Object[]; IsExperimentalVersion=False}.DocsURL)

Branch quality note: keep this plugin folder self-contained and verify that the branch opens with TinyToolDev_Examples.uproject.
