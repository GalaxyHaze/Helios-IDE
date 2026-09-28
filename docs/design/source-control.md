# Source control

## Intent

Source Control is a quiet, task-oriented surface for understanding the
workspace's Git state and acting on it without leaving the editor. It should
make the current branch, changed files, and next safe action legible before
exposing secondary operations.

## Decisions

- The panel header establishes the Source Control identity, current branch,
  and an explicit refresh action.
- The changed-file list is the primary surface; commit input and staging
  actions remain above it so the action path stays visible while the list
  grows.
- Each row shows a compact status badge, the file name, and a subdued
  relative path. The status badge carries meaning through both color and text.
- Selection is multi-select and is preserved by relative path when the list is
  refreshed or the theme changes.
- A clean repository is represented by a concise empty-state message rather
  than an empty unexplained list.
- Missing repository and missing remote states expose an inline next action
  for initialization or remote connection; they do not open a modal
  automatically.
- Busy operations disable mutating controls while keeping the current state
  visible.
- Activating a row opens the corresponding file in the editor when the path
  still exists.

## Invariants

- The visible list represents one complete Git status snapshot; it is never an
  append-only event log.
- Branch identity remains visible while the list is refreshed.
- Theme changes restyle the existing snapshot without starting another Git
  operation.
- Destructive or broad actions such as Stage all remain visually distinct from
  selection-scoped actions.
- Error and informational messages occupy the same summary location and do not
  displace the changed-file list.

## Future review questions

- If diff previews are added, do they belong in a transient adjacent surface or
  in the editor workspace?
- Does remote synchronization add a separate task state, or can it reuse the
  existing busy and summary vocabulary?
- Does a commit history view deserve a separate aspect, or is it still part of
  Source Control?
