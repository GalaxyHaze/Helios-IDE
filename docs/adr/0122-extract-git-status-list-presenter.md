---
status: accepted
---

# ADR-0122: Extract the Git status list presenter

## Context

`GitPanel` had two different responsibilities inside the same widget:
coordinating repository commands through `GitRepositorySession`, and
constructing the visual snapshot of changed files. The latter included
status-badge semantics, row layout, theme projection, selection restoration,
and conversion from selected rows back to relative paths.

That code made changes to the source-control visual language or selection
behavior require editing the command-oriented panel. It also made the
snapshot projection difficult to exercise without triggering repository
operations.

## Decision

Introduce `GitStatusListPresenter` as the presentation module for the
changed-file list. Its interface is intentionally small:

- `render(state)` replaces the list with one complete repository snapshot and
  restores selection by relative path;
- `applyTheme()` reprojects the retained snapshot without starting Git;
- `selectedRelativePaths()` returns the command input needed by stage/unstage;
- `clear()` removes the visible snapshot at an explicit refresh/teardown
  boundary.

`GitPanel` remains the adapter for user actions, repository messages, branch
and summary labels, busy-state controls, and file activation. The presenter
does not own Git process state, command policy, or navigation.

## Alternatives considered

### Keep row construction in `GitPanel`

Rejected because the panel would continue to combine command orchestration
with a sizeable snapshot projection and theme-specific row policy.

### Move the entire source-control panel into a generic controller

Rejected because it would create a shallow context object with callbacks for
every button and label. The useful invariant is specifically the complete
status snapshot and selection preservation.

### Use a model/view abstraction immediately

Rejected for now because the current list is small and widget-backed. The
presenter creates a seam without committing the rest of the panel to a new
model architecture; a model can be introduced later if scale or diff
preview requirements justify it.

## Consequences

- Status-row layout and styling are local to one module.
- Refresh and theme changes preserve selection by the stable relative path,
  not by list index.
- The panel has a narrower dependency on the list: it asks for selected
  command paths and delegates snapshot projection.
- The presenter is still deliberately UI-specific; it is not a repository
  abstraction and should not absorb Git command behavior.

## Verification

`testGitStatusListPresenterProjectsSnapshotAndSelection` covers snapshot
replacement, stable selection, theme reprojection, and explicit clearing.
`testGitPanelReplacesStatusSnapshotAndRendersThemeAgain` covers integration
with the repository session and panel adapter.
