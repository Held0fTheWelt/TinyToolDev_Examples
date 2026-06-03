# Plugins Folder

The base branch does not require a product-specific plugin.

Example branches can use this folder in one of two ways:

- Include a plugin copy when the license and distribution model allow it.
- Leave the folder empty and document where the buyer should install the plugin.

Every branch that depends on a plugin should say:

- Which plugin is required.
- Which plugin version was used while testing.
- Whether the plugin is bundled or must be installed separately.
- Where the product documentation is linked in the plugin `.uplugin`.

Never commit private marketplace packages, local-only builds, or credentials.
