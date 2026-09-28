# ADR-0110: Render Git status from the latest snapshot

- Status: Accepted
- Date: 2026-09-27

## Context

`GitRepositorySession` emits a complete `GitStatusSnapshot`, but `GitPanel`
previously appended rows directly while handling each emission. Repeated
refreshes could therefore duplicate files. Theme changes also cleared child
styles without rebuilding the row widgets, leaving status badges with stale or
missing visual roles.

The panel needs a stable presentation invariant: the visible list is a
projection of exactly one latest snapshot, not an event log.

## Decision

`GitPanel` stores the latest `GitStatusSnapshot` and renders it through one
replacement operation:

- receiving a snapshot replaces the current row projection;
- rendering clears the list before creating rows;
- theme application re-renders the stored snapshot after applying base styles;
- selected relative paths are captured and restored across a re-render.

`GitRepositorySession` remains the owner of Git process sequencing,
availability, and snapshot production. The panel owns only the widget
projection and user interaction.

## Alternatives considered

### Append rows for every status signal

Rejected because a status signal represents a new complete snapshot, not one
additional row. Append semantics make refresh correctness depend on external
deduplication.

### Ask the session to emit row-level mutations

Rejected because it would couple the source-control workflow to Qt item
widgets and make theme re-rendering impossible without replaying operations.

### Re-fetch Git status on every theme change

Rejected because theme application is a presentation operation and should not
start a process or depend on repository availability. Reusing the stored
snapshot keeps the operation local and deterministic.

## Consequences

- Refreshes are idempotent at the presentation seam.
- Theme changes update status rows without another Git command.
- Selection is preserved by relative path when rows are rebuilt.
- Row rendering remains inside `GitPanel`; no shallow generic list adapter is
  introduced.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testGitPanelReplacesStatusSnapshotAndRendersThemeAgain -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
