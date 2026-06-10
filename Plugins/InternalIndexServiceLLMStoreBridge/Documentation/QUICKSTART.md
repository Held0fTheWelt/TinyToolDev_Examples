# Quickstart

## 1. Install Required Plugins

Install these plugins side by side:

```text
Plugins/
  InternalIndexService/
  LLMStore/
  InternalIndexServiceLLMStoreBridge/
```

The bridge is not useful by itself. It requires both IIS and LLM Store.

## 2. Enable Plugins

Enable all three in your `.uproject` or through the Unreal Plugin Browser:

```json
{
  "Name": "InternalIndexService",
  "Enabled": true
},
{
  "Name": "LLMStore",
  "Enabled": true
},
{
  "Name": "InternalIndexServiceLLMStoreBridge",
  "Enabled": true
}
```

## 3. Build

For a project build, use your normal Unreal build flow.

For plugin packaging, build in a host that also contains IIS and LLM Store, because this bridge depends on their public interface modules:

```bat
D:\Engines\UE_5.4\Engine\Binaries\ThirdParty\DotNet\6.0.302\windows\dotnet.exe ^
  D:\Engines\UE_5.4\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll ^
  UnrealEditor Win64 Development ^
  -Project=D:\YourHost\YourHost.uproject ^
  -NoHotReload -NoUBTMakefiles
```

## 4. Verify Startup

Start the editor or run `UnrealEditor-Cmd`.

Expected log:

```text
IIS LLM Store Bridge registered embedding executor 'llmstore'.
```

If the bridge is disabled, IIS still works for import, catalog, lexical search, context packs, and agent evidence tools. Embedding execution will fail with a controlled missing-executor error.

## 5. Run an IIS Embedding Flow

In IIS:

1. Import prepared chunks.
2. Build the chunk catalog.
3. Build embedding jobs.
4. Execute embedding jobs.

When the bridge is active, IIS calls the registered executor. The bridge calls LLM Store embedding routes. LLM Store decides provider, model, runtime mode, local-only policy, and fallbacks.

## 5a. Blueprint Smoke Test

In an Editor Utility Blueprint, use:

```text
Is IIS LLM Store Bridge Registered
Resolve IIS Embedding Route via LLM Store
Execute Simple IIS Embedding via LLM Store
```

See [Blueprint usage](BLUEPRINT_USAGE.md) for node-by-node examples.

## 6. Expected Default Routes

LLM Store should expose these IIS route names:

```text
iis.embedding.code
iis.embedding.blueprint
iis.embedding.asset
```

For local test runs, LLM Store may route them to its mock embedding provider.
