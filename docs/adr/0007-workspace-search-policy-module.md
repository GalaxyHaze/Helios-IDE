---
status: accepted
---

# Workspace search policy module

Workspace search and replacement policy lives in the `WorkspaceSearch` module,
not in the `SearchPanel` widget or `MainWindow`. The module owns file-filter
rules, case-insensitive replacement edit generation, and application of edit
plans to text. It has no widget, thread-pool, or settings-store dependency.

The Qt panel remains an adapter for input, result presentation, asynchronous
file traversal, and confirmation. Search settings are read on the UI thread
and passed as explicit filter data to worker runnables; workers do not access
the global settings store.

## Considered options

- Keep replacement policy in `SearchPanel`: rejected because workspace replace
  is also consumed by `MainWindow`, and policy tests would require constructing
  a widget.
- Move all filesystem traversal into the domain module immediately: deferred
  because cancellation, batching, and Qt delivery are UI integration concerns
  with only one current adapter.
- Let worker threads read `TomlSettingsStore`: rejected because it hides a
  mutable global dependency in asynchronous code.

## Consequences

- Replacement behavior has one testable seam and cannot diverge between the
  search panel and workspace application path.
- Changing the UI or replacing the Qt worker adapter does not change search
  semantics.
- The next search-related extraction can move traversal behind an explicit
  cancellation interface if a second adapter or stronger reuse need appears.
