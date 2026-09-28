# ADR-0119: Make editor chrome an explicit collaborator

- Status: Accepted
- Date: 2026-09-27

## Context

`EditorSessionController`, `EditorInteractionController`, and
`WorkspacePanelPresentationController` all needed to refresh the same editor
presentation. Each module received a generic `updateEditorChrome` callback from
`MainWindow`, so the composition root acted as a pass-through adapter between
the callers and `EditorChromeController`.

That interface hid the semantic dependency and duplicated the same wiring in
three places. It also kept `MainWindow` responsible for forwarding a concern
that already had a deep module with a stable `update(CodeEditor *)` interface.

## Decision

Inject `EditorChromeController` explicitly into the modules that need to
refresh editor presentation. Those modules call its interface directly;
`MainWindow` remains responsible for constructing the controller and supplying
it as the composition root.

The editor chrome module continues to own the derived presentation state:
breadcrumbs, cursor position, language label, window title, find/replace
target, and outline request identity. The collaborating modules do not
reimplement that policy.

## Alternatives considered

### Keep a generic `updateEditorChrome` callback

Rejected because it is a shallow pass-through seam. Every caller must learn a
callback whose meaning is only clear from `MainWindow`, and the same adapter is
duplicated across the composition root.

### Add a new `EditorPresentationCoordinator`

Rejected because it would add another wrapper around the existing deep
`EditorChromeController` without a second implementation or a varying adapter.

### Move all editor-refresh triggers into `EditorChromeController`

Rejected for now because session lifecycle, Vim interaction, and workspace
panel visibility remain owned by their respective modules. They should request
an update through the explicit seam, not transfer their policies into the
presentation module.

## Consequences

- Editor presentation has one named collaborator instead of three generic
  callbacks.
- `MainWindow` no longer exposes or forwards `updateEditorChrome`.
- Tests can provide a real `EditorChromeController` with lightweight adapters
  and verify the presentation seam without constructing `MainWindow`.
- The lifecycle modules still decide when an update is needed; the chrome
  module remains the single owner of how presentation is derived.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
