# IIS LLM Store Bridge Documentation

## Audience

This documentation is for teams that have both Internal Index Service and LLM Store installed and want IIS embedding jobs to use governed LLM Store embedding routes.

## Documents

- [Quickstart](QUICKSTART.md): install, enable, build, and verify the bridge.
- [Blueprint usage](BLUEPRINT_USAGE.md): Blueprint nodes, diagnostic graphs, and embedding smoke-test recipes.
- [Buyer guide](BUYER_GUIDE.md): who benefits from the bridge and where the product boundary sits.
- [Integration details](INTEGRATION.md): runtime architecture, interfaces, route flow, and boundaries.
- [Source overview](SOURCE_OVERVIEW.md): source file map, executor lifetime, and request/response mapping.
- [Troubleshooting](TROUBLESHOOTING.md): common setup and runtime failures.
- [Release checklist](RELEASE_CHECKLIST.md): checks before publishing or packaging this bridge.

## Boundary Summary

The bridge is intentionally small.

```text
Internal Index Service
  owns indexing and embedding job orchestration

IIS LLM Store Bridge
  owns request/response adaptation

LLM Store
  owns provider routing, models, governance, secrets, and fallbacks
```

No plugin silently bypasses another plugin's responsibility.
