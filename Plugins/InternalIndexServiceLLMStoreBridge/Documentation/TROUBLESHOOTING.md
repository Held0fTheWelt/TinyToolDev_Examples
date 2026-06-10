# Troubleshooting

## Bridge Does Not Register

Expected startup log:

```text
IIS LLM Store Bridge registered embedding executor 'llmstore'.
```

If it is missing:

- confirm the plugin is enabled;
- confirm IIS is enabled;
- confirm LLM Store is enabled;
- confirm the module loads after engine initialization;
- check for compile or module-load errors earlier in the log.

## IIS Reports `embedding_executor_not_registered`

IIS can run without this bridge. That error means no embedding executor is registered.

Fix:

- enable `InternalIndexServiceLLMStoreBridge`;
- rebuild the project;
- restart the editor;
- verify the startup log.

## LLM Store Cannot Be Resolved

Possible bridge error:

```text
LLM Store subsystem class was not found or is not an engine subsystem.
```

Fix:

- enable the `LLMStore` plugin;
- verify the LLM Store runtime module is compiled;
- verify the LLM Store subsystem is available in the current build target.

## Route Cannot Be Resolved

Possible bridge warning:

```text
LLM Store could not resolve embedding route 'iis.embedding.code'.
```

Fix:

- configure the IIS embedding routes in LLM Store;
- verify the route is enabled;
- verify the active runtime mode allows the route;
- verify the selected provider supports the configured embedding model.

Recommended routes:

```text
iis.embedding.code
iis.embedding.blueprint
iis.embedding.asset
```

## Route Blocked by Governance

This is expected when LLM Store policy blocks execution.

Check:

- route enabled flag;
- runtime mode (`mock`, `local-only`, `fast`, `deep`, `disabled`);
- local-only constraint;
- provider allow list;
- model allow list;
- fallback policy.

The bridge should not bypass this. Update LLM Store policy instead.

## BuildPlugin Fails Because Dependencies Are Missing

`RunUAT BuildPlugin` builds one plugin in a temporary host. This bridge needs IIS and LLM Store public interface modules in that host.

Use a short host project that contains:

```text
Plugins/InternalIndexService/
Plugins/LLMStore/
Plugins/InternalIndexServiceLLMStoreBridge/
```

Then build the host target or package from that host.

## Do Not Commit Build Artifacts

Public repositories should not include generated folders:

```text
Binaries/
Intermediate/
Saved/
DerivedDataCache/
```

If those appear after a local build, delete them before publishing.

