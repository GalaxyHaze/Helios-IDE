# ADR-0118: Require explicit save success before closing a tab

- Status: Accepted
- Date: 2026-09-27

## Context

The tab-close controller and the Vim `wq` interaction asked adapters to save a
modified editor, but their callbacks returned no result. The controllers could
only infer success from the widget's modified flag, which mixed the editor's
state with the adapter's operation and made cancellation or write failure an
implicit contract.

## Decision

The tab-close seam requires `saveEditor` to return `bool`, and the editor
interaction seam requires `saveCurrent` to return `bool`. A save decision or
`wq` command can close the editor only when the adapter reports success and the
document is no longer modified. The document-state check remains a defensive
invariant, while the adapter result is the explicit operation outcome.

## Alternatives considered

### Keep a `void` save callback and inspect only the document

Rejected because the controller cannot distinguish a failed save from an
adapter that forgot to clear the modified state.

### Let the close controller own file persistence

Rejected because file dialogs, paths, and persistence belong to the editor-file
module; the close controller should coordinate the decision, not implement
storage.

### Let `wq` infer success from the document modified flag

Rejected because a failed save can leave the flag unchanged, while an adapter
may also have side effects that are not represented by the widget flag.

## Consequences

- Failed or cancelled saves leave the tab open.
- Failed `wq` saves leave the current editor open.
- The adapter contract is testable without relying solely on Qt document flags.
- The close controller remains independent of file-dialog and persistence
  details.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testEditorTabCloseControllerPreservesSaveOrdering testEditorTabCloseControllerKeepsTabWhenSaveFails -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
