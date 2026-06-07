# Internal Index Service Parallel Work Handoff

This note exists because IIS is being built while UII/IIS import work may happen in parallel.

## Current task boundary

The active skeleton task owns only:

- `InternalIndexService.uplugin`
- `Source/InternalIndexServiceInterface`
- `Source/InternalIndexService`
- `Source/InternalIndexServiceEditor`
- storage path helpers
- service stubs
- editor menu stubs
- Python smoke-test bridge

The skeleton task must not introduce:

- UII import execution
- LLM-Store or provider dependencies
- embedding execution
- SQLite, FTS, vector stores, hybrid search, or MCP tools
- project-specific rules or sample-specific strings

## Parallel UII import prototype

UII-to-IIS import or docking work is valid, but it belongs to a later task such as
`IIS MVP B: Import UII Prepared Chunks JSONL into Local Chunk Store`.

Prototype files that appeared during the skeleton task were moved out of compiled
`Source/` paths and, as of the production-readiness pass, out of the plugin package
root as well:

- `docs/superpowers/parallel-work/InternalIndexService/UIIImportPrototype/`

Those files are intentionally build-neutral and package-neutral. A later task can
promote them back into `Source/` only after updating the IIS boundary,
dependencies, tests, and validation scope for that task.

## Working method for parallel agents

- Do not add LLM-Store dependencies to `InternalIndexServiceInterface` during the skeleton phase.
- Do not duplicate contract structs already defined in `IISIndexTypes.h`, `IISSearchTypes.h`, or `IISContextPackTypes.h`.
- Do not add import/menu commands that execute UII import during the skeleton phase.
- If parallel work needs a prototype, park it under
  `docs/superpowers/parallel-work/InternalIndexService/<task-name>/`.
- Before promoting a prototype into `Source/`, run `BuildPlugin` and editor smoke validation.

The intended sequence remains:

1. IIS Plugin Skeleton + Index Contract.
2. IIS MVP B: Import UII Prepared Chunks JSONL into Local Chunk Store.
3. IIS MVP C: SQLite FTS / BM25 Search.
4. IIS MVP D: Embedding integration via LLM-Store.
