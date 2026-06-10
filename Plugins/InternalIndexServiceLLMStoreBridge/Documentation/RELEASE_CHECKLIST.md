# Release Checklist

Use this checklist before publishing or packaging the bridge.

## Repository Hygiene

- `Binaries/` is absent.
- `Intermediate/` is absent.
- `Saved/` is absent.
- No local build logs are committed.
- No provider secrets, API keys, or local endpoint tokens are committed.
- Documentation is included:
  - `README.md`
  - `Documentation/README.md`
  - `Documentation/BUYER_GUIDE.md`
  - `Documentation/QUICKSTART.md`
  - `Documentation/INTEGRATION.md`
  - `Documentation/TROUBLESHOOTING.md`

## Boundary Checks

- IIS does not depend on LLM Store.
- LLM Store does not depend on IIS.
- The bridge is the only plugin that depends on both public interface modules.
- The bridge does not call provider SDKs directly.
- The bridge does not store embeddings.
- The bridge does not execute search.
- The bridge does not execute chat/completion routes.
- The bridge does not mutate assets, Blueprints, source files, or project settings.

## Build Checks

Build in a host project containing IIS, LLM Store, and the bridge:

```bat
D:\Engines\UE_5.4\Engine\Binaries\ThirdParty\DotNet\6.0.302\windows\dotnet.exe ^
  D:\Engines\UE_5.4\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll ^
  UnrealEditor Win64 Development ^
  -Project=D:\YourHost\YourHost.uproject ^
  -NoHotReload -NoUBTMakefiles
```

Expected:

- `UnrealEditor-InternalIndexServiceLLMStoreBridge.dll` builds.
- No bridge compile errors.
- Startup log registers executor `llmstore`.

## Functional Smoke Check

With IIS and LLM Store enabled:

1. Build IIS embedding jobs.
2. Execute a small number of jobs.
3. Confirm IIS vector output contains provider/model metadata from LLM Store.
4. Disable the route in LLM Store and confirm IIS receives a controlled governance failure.

## Public Description

Use wording like:

```text
Optional companion bridge for users who have both Internal Index Service and LLM Store.
It lets IIS embedding jobs use governed LLM Store embedding routes without coupling either core plugin.
```

Avoid wording that implies the bridge includes IIS, LLM Store, model providers, or provider credentials.
