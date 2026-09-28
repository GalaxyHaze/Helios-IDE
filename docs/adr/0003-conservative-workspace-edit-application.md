---
status: accepted
---

# Conservative workspace edit application

Helios accepts only the LSP `changes` form of a workspace edit and only local-file URIs. It validates every target range before mutating any document; open targets are edited in memory, while closed targets are written through `QSaveFile`. `documentChanges` and non-local targets are rejected rather than partially interpreted.

## Considered options

- Apply edits as they arrive: rejected because a later invalid range could leave earlier targets partially changed.
- Support every LSP workspace-edit form immediately: deferred because the current IDE needs a safe local-file path before broader protocol coverage.

## Consequences

- A workspace edit either passes the current validation phase or no target is intentionally changed.
- Open and closed documents have different post-edit state: open documents become modified, while closed files are saved.
- Range conversion, ordering, and atomicity are architectural concerns that should remain behind one edit-application seam.
