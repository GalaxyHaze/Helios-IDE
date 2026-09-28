# ADR-0107: Extract the workspace search session

- Status: Accepted
- Date: 2026-09-27

## Context

`SearchPanel` was both a Qt presentation surface and the owner of the
asynchronous workspace scanner. Its implementation contained two
`QRunnable`s, directory traversal, text-extension filtering, result batching,
replace-preview construction, and an atomic token used to suppress stale
results.

That placement made the panel responsible for a concurrency invariant that
was unrelated to its layout: after a new search or root change, callbacks from
an older scan must not mutate the visible result list. It also made the
replace-preview path depend on panel lifetime details.

## Decision

Introduce `WorkspaceSearchController` as the module for one asynchronous
workspace search session. Its interface is:

- `setRootPath` to establish the scan root and cancel an older session;
- `search` to start a query;
- `previewReplace` to scan replacement targets;
- `cancel` to invalidate outstanding work.

The controller samples the supplied `ScanPolicy` when a session starts,
performs traversal and batching on the thread pool, and emits only
current-session results. The session token remains private; consumers receive
results and completion state, not a cancellation mechanism or token they must
interpret.

`SearchPanel` retains ownership of debounce, input validation, result widgets,
summary text, and item activation. `WorkspaceSearch` remains the pure
text/range policy module, while `WorkspaceReplaceController` remains the
mutation module.

## Alternatives considered

### Keep workers inside `SearchPanel`

Rejected because widget lifetime and scan cancellation would remain coupled,
and every new search surface would have to reproduce the concurrency policy.

### Put asynchronous scanning into `WorkspaceSearch`

Rejected because `WorkspaceSearch` is deliberately pure and synchronous. Adding
thread-pool ownership and Qt signals there would mix text policy with
application scheduling.

### Expose the atomic token to `SearchPanel`

Rejected because it leaks an implementation invariant. The controller can
guarantee that stale results are discarded without making every caller compare
tokens.

## Consequences

- Search and replacement preview concurrency has one locality.
- `SearchPanel` is a presentation adapter with a smaller implementation.
- The controller can be tested without constructing the full panel.
- A future search surface can reuse the same session policy.
- The controller still uses Qt's global thread pool; changing scheduling is a
  future implementation decision, not part of its consumer interface.

## Verification

- `cmake --build build -j 2`
- `./build/test_helios testWorkspaceSearchControllerOwnsAsyncScanAndPreview -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
