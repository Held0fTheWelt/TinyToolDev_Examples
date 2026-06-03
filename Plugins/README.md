# Plugins Folder

This branch requires the separately distributed `LLMStore` plugin. The plugin package itself is intentionally not part of this example repository.

Install it locally in one of these ways:

- Copy or install the plugin to `Plugins/LLMStore`.
- Install it as an engine plugin available to Unreal Engine 5.4.

`Plugins/LLMStore` is intentionally ignored by `.gitignore`. That keeps locally installed plugin packages, private builds, generated binaries, credentials, and machine-specific files out of this example branch.

This is also the intended maintainer workflow for checking the examples with the original plugin: install or copy the plugin here locally, run the sample validation, and keep the plugin package out of Git.

Public plugin documentation:

https://github.com/Held0fTheWelt/TinyToolDevelopment/tree/master/ai-plugins/LLMStore

Do not commit API keys, local provider credentials, private marketplace packages, or generated binaries.
