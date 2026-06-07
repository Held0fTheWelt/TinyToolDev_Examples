# Changelog

All notable changes to this plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/de/1.0.0/),
and this plugin adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Added Fab third-party software declaration notes for bundled `hnswlib`.
- Added `UIISSettings` (`UDeveloperSettings`) as the project-settings surface for vector backend and MCP endpoint configuration (Plugins → Internal Index Service).
- Added Control Panel v2 host chrome: retrieval-only header badge, global Refresh, and active-state tab styling.
- Added dashboard-first Control Panel landing tab with health cards for Index, MCP Server, and Integrations (`IISCapturePanelStatus`, `SIISStatusCard`, `SIISDashboard`).
- Added editor-module MCP lifecycle API (`StartMcp`, `StopMcp`, `RotateMcpToken`) so the dashboard can start and stop the loopback endpoint from the UI.
- Added incremental-index lifecycle helpers, vector-store rewrite on prune, and HNSW index cleanup when vectors are removed.
- Added release validation gate tooling (`run_iis_gate.py`) with `release` profile stages (`automation_full`, `sample_scan`, `mutation_flags`) and local override example config.
- Added automation coverage for settings defaults, panel status, dashboard/widgets, MCP lifecycle, and incremental conflict/rollback scenarios.
- Added Control Panel v2 **Governance** tab (retrieval-only invariant display, MCP security, data locality, bridge health, agent transparency).
- Added in-panel **Settings** tab mirroring `UIISSettings` with Project Settings shortcut.
- Added grouped tabs: Dashboard | Index | Use | Agents | Governance | Diagnostics | Settings.
- Added dashboard work row (Import shortcut, quick Search, open index folder) with explicit refresh/update actions.
- Added optional `IndexRoot` on `UIISSettings` (empty = default `Saved/InternalIndexService`).
- Slimmed **Tools → Internal Index Service** menu to Open panel + quick actions (Build Index, Start/Stop MCP, Open Index Folder).

### Changed
- Updated `Config/FilterPlugin.ini` to include the `Tools/...` folder in packaged Fab builds while excluding local Python cache/config artifacts.
- MCP endpoint and vector backend resolution now read from `UIISSettings` instead of ad-hoc `GConfig` access in editor/runtime paths.
- `FIISStoragePaths::GetDefaultIndexRoot()` respects `UIISSettings::IndexRoot` when set.
- Search and catalog operations filter non-active chunk lifecycle states consistently.
- Dashboard health cards now use persistent widgets and text-only updates; they refresh on open, explicit Refresh, and dashboard actions, removing the remaining visible flicker path.
- Dashboard chunk status now reads the canonical catalog build report from the index folder, with legacy report-folder fallback.
- Replaced non-portable status/header glyphs and cached localized UI labels with direct plain-text labels so unsupported fonts do not show replacement characters.

## [0.1.0] - 2026-05-31

### Added
- Added interface, runtime, and editor modules for local indexing, retrieval, and context-pack workflows.
- Added public contracts for chunks, symbols, assets, Blueprint graph records, search queries, search results, imports, context packs, and agent access.
- Added local storage path handling for project-local Internal Index Service artifacts.
- Added UII evidence import and prepared chunk import support.
- Added chunk catalog and local index service foundations for lexical search and context-pack assembly.
- Added an embedding job queue that delegates provider, model, route, and governance decisions to LLM Store.
- Added durable vector output artifacts, embedding run reports, editor menu actions, and Python bridge functions for embedding operations.

### Changed
- Defined plugin boundaries so IIS owns indexing and retrieval while UII owns Unreal evidence extraction and LLM Store owns model routing and secrets.
