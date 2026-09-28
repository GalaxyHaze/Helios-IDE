---
status: accepted
---

# ADR-0121: Preserve pending document synchronization until delivery or baseline

## Context

`EditorDocumentSyncController` debounces local changes before sending them to
the language service. A client can be temporarily unavailable while the
editor remains alive, for example while a language server is starting or
restarting. Treating that condition as a discard caused local changes to be
removed from the pending batch without the server ever receiving them.

There is a second transition when a client becomes ready: the coordinator
opens the document with its current full text. If changes accumulated before
readiness were retained but not explicitly reconciled with that open snapshot,
the same changes could be sent a second time after `didOpen`.

## Decision

The synchronization seam distinguishes three operations:

- `flush` attempts delivery and preserves pending changes when the transport
  is unavailable or the required sender callback is missing;
- `markDocumentSynchronized` establishes that the current local text and
  version were represented by the just-sent open snapshot, so pending changes
  already included in that snapshot can be cleared;
- `discardPendingChanges` remains an explicit teardown operation used when a
  document is detached or closed.

`LspDocumentCoordinator` calls the baseline operation immediately after
opening the current editor text on a ready client. The editor remains the
source of truth for text, while the synchronization controller owns the
pending delivery state and the coordinator owns the open/close transition.

## Alternatives considered

### Discard on every unavailable flush

Rejected because readiness is transient and a local edit must not disappear
from the language-service stream merely because the process is starting.

### Replay pending changes after `didOpen`

Rejected because `didOpen` already carries the current full text. Replaying
the same edits would duplicate content or advance the protocol version
without representing a new local edit.

### Move pending state into `LspDocumentProtocol`

Rejected because the pending state is editor-side debounce and delivery
policy. The protocol module should serialize notifications and track the
server-facing version registry, not own Qt timing or local edit capture.

## Consequences

- A temporary language-service outage no longer loses queued editor changes.
- Reconnection has an explicit baseline transition instead of relying on an
  incidental queue clear.
- Teardown must continue to call the explicit discard operation.
- The sender callbacks remain deliberately small; retry scheduling is still
  controlled by editor lifecycle events rather than a generic message bus.

## Verification

`testEditorDocumentSyncControllerOwnsBatchingPolicy` verifies both retry after
temporary unavailability and clearing after an explicit synchronized
baseline. The full Qt test suite covers the remaining editor and LSP seams.
