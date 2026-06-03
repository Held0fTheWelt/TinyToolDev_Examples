# Plugins Folder

This branch requires the separately distributed `SmartContentDiet` plugin. The plugin package itself is intentionally not part of this example repository.

Install it locally in one of these ways:

- Copy or install the plugin to `Plugins/SmartContentDiet`.
- Install it as an engine plugin available to Unreal Engine 5.4.

`Plugins/SmartContentDiet` is intentionally ignored by `.gitignore`. That keeps locally installed plugin packages, private builds, generated binaries, credentials, and machine-specific files out of this example branch.

This is also the intended maintainer workflow for checking the examples with the original plugin: install or copy the plugin here locally, run the sample validation, and keep the plugin package out of Git.

Maintainer copy path from the development monorepo:

```text
copy <TinyToolDevelopment>/Git/GovernanceDevelopmentPlugins/SmartContentDiet  →  Plugins/SmartContentDiet
```

Public plugin documentation:

https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/GovernanceDevelopmentPlugins/SmartContentDiet

Do not commit API keys, local credentials, private marketplace packages, or generated binaries.
