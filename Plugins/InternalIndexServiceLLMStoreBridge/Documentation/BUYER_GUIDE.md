# Buyer Guide

## What This Plugin Is

IIS LLM Store Bridge is an optional companion plugin for users who have both:

- **Internal Index Service**; and
- **LLM Store**.

It is intentionally small. Its job is to connect the IIS embedding executor interface to LLM Store embedding routes.

## Why It Exists

Without the bridge, IIS can still import evidence, build catalogs, run lexical retrieval, expose agent evidence tools, and manage local index artifacts.

With the bridge, IIS embedding jobs can use LLM Store governance:

- route-based provider selection;
- local-only model routing;
- runtime modes such as `mock`, `fast`, `deep`, and `disabled`;
- provider/model fallback policy;
- centralized secret handling;
- deterministic mock embeddings for tests.

This keeps IIS clean: IIS indexes evidence, while LLM Store governs model execution.

## Who Should Use It

Use this bridge if:

- you own and use both IIS and LLM Store;
- you want IIS embeddings to follow LLM Store route policy;
- you do not want provider/model IDs hardcoded into IIS;
- you need local-first or fully local embedding workflows;
- you want testable mock embedding execution without external provider dependencies.

You do not need this bridge if:

- you only use IIS for lexical retrieval;
- you have not installed LLM Store;
- you want to write a custom IIS embedding executor for another model broker.

## Product Boundary

This repository does not include IIS or LLM Store source code. It expects both plugins to be installed separately.

The bridge is a benefit layer for combined installations, not a standalone indexing system and not a standalone model broker.

