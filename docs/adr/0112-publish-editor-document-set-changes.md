# ADR-0112: Publish editor document-set changes

- Status: Accepted
- Date: 2026-09-27

## Context

`EditorSessionController` previously accepted an
`updateClangdLifecycle` callback and invoked it when a file was opened,
assigned a path, or released. That made a document-session module know about a
specific language-service lifecycle policy. It also increased the callback
surface passed from `MainWindow`.

The meaningful event is not “update clangd”; it is that the set or identity of
open editor documents changed. The composition root and language-service
workspace controller should decide what that event means for reconciliation.

## Decision

Add the semantic `documentSetChanged` signal to `EditorSessionController`.
Emit it after:

- a new file path has been opened into the session;
- an untitled editor receives a persistent path;
- an editor is released.

`MainWindow` connects this signal to its existing clangd reconciliation
adapter. `EditorSessionController` no longer receives an
`updateClangdLifecycle` callback.

When a persisted session is restored, the controller batches the internal
close/open transitions and emits one signal after the resulting document set
is materialized. Consumers therefore reconcile the final topology rather than
observing each intermediate tab.

The signal does not replace document synchronization. Opening, changing,
saving, and closing protocol documents remain the responsibility of
`LspDocumentCoordinator`; `documentSetChanged` only reports a session
topology/identity change that may affect language-service routing.

## Alternatives considered

### Keep the lifecycle callback

Rejected because the session would continue to depend on one consumer's policy
and the callback name would encode clangd rather than the domain event.

### Make `EditorSessionController` call `reconcile()` directly

Rejected because it would couple the document session to workspace settings,
clangd discovery, and runtime lifecycle.

### Emit a signal for every tab mutation

Rejected because creating an untitled tab does not change language routing.
The signal is emitted for path/open/close transitions that can change the
language-service document set.

## Consequences

- The session interface names a domain event instead of a consumer action.
- `MainWindow` remains the composition root for language-service policy.
- The callback bag is smaller and easier to reuse in tests.
- A future language service can subscribe without changing the session module.
- The event is intentionally coarse; document content/version changes remain
  in the synchronization seam.
- Session restoration does not expose intermediate document topologies to
  subscribers.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testEditorSessionControllerCapturesAndRestoresDocuments -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testEditorSessionControllerOwnsSavePolicy -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testEditorSessionControllerPublishesDocumentIdentityTransitions -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
