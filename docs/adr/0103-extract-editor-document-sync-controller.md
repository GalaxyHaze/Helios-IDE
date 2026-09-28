# ADR-0103: Extract editor document synchronization

- Status: Accepted
- Date: 2026-09-27

## Context

`CodeEditor` has to observe `QTextDocument` changes, but it should not also
know the debounce interval, pending-batch lifecycle, server synchronization
kind, or the protocol-specific choice between ranged and full-document
notifications.

The lower-level `LspDocumentSync` already owns versioned text changes and
position conversion. The remaining editor-side policy was still embedded in
the widget and coupled text editing to `LspClient` calls.

## Decision

Introduce `EditorDocumentSyncController` as the editor-side module that owns
the debounce timer, pending batch lifecycle, version progression, and
incremental-versus-full dispatch decision.

The module receives transport-facing callbacks for availability and sending.
`CodeEditor` remains responsible for extracting the changed text from Qt's
document model and for deciding when programmatic document replacement should
be suppressed. The controller does not depend on `LspClient` or on Qt editor
widgets.

The existing `LspDocumentSync` remains the protocol value module underneath
the controller. This keeps protocol range/version rules separate from the
editor's timing and transport policy.

## Alternatives considered

### Keep the policy in `CodeEditor`

This keeps the code close to the text widget but makes every synchronization
change require editing a large input/presentation class. It also makes
incremental/full dispatch harder to test without constructing the editor.

### Put debounce behavior in `LspDocumentSync`

That would mix a pure document value model with QObject timing and transport
availability. It would make the protocol module less reusable and less
predictable.

### Add a generic event or message bus

The synchronization flow is ordered and document-specific. A bus would hide
the version invariant rather than provide a deeper interface for it.

## Consequences

- The editor widget no longer knows the LSP synchronization kind or dispatch
  methods.
- Batching and full-sync fallback can be tested through callbacks.
- The controller's interface is editor-document specific rather than a
  generic transport abstraction.
- The Qt document remains the source of truth for extracting local changes;
  the controller owns only synchronization state and delivery policy.
