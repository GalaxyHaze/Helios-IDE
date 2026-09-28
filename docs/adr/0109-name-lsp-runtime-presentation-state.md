# ADR-0109: Name the LSP runtime presentation state

- Status: Accepted
- Date: 2026-09-27

## Context

`LspRuntimePresentationController` projected the same runtime information into
`SettingsPanel` and `LspManagerDialog` through several positional `QString`
arguments. Runtime status, clangd status, and LSP diagnostics each had a
different group of same-typed values, so a reordered or duplicated argument
could compile while presenting the wrong field.

The two panels are distinct presentation surfaces, but they consume the same
semantic snapshots. Keeping the state implicit in parameter order made the
shared interface less precise and forced the presenter to repeat the
projection shape at every consumer.

## Decision

Introduce `LspRuntimePresentationState.h` with three value types:

- `LspRuntimeInfo` for the managed Zith runtime;
- `ClangdInfo` for the C-family language service;
- `LspDiagnosticsInfo` for connection, synchronization, and last-error state.

`SettingsPanel` and `LspManagerDialog` accept these named values. The
`LspRuntimePresentationController` constructs each snapshot once and projects
the same value to both surfaces. The values contain presentation data only;
they do not own lifecycle, persistence, widgets, or process state.

This is an interface improvement, not a generic model layer. Each value
contains only fields that already crossed the seam together, and the panels
remain responsible for their own fallback text, translation, and layout.

## Alternatives considered

### Keep positional strings and add comments

Rejected because comments do not make an invalid ordering unrepresentable.
The compiler should expose the semantic grouping at the call site.

### Create one shared widget for both panels

Rejected because the settings panel and manager dialog have different
interaction surfaces and lifecycles. Sharing the value interface gives
leverage without forcing their layouts into one adapter.

### Introduce a persistent global runtime view model

Rejected because these values are snapshots for presentation, not an
application-owned source of truth. Lifecycle coordinators and settings remain
the owners of runtime state.

## Consequences

- Runtime presentation calls are self-describing and harder to misuse.
- Both panels receive exactly the same semantic snapshot.
- The presenter owns state derivation; panels own formatting and fallback text.
- Adding a runtime field requires updating one value type and its explicit
  consumers instead of relying on positional conventions.
- The values are intentionally non-`QObject` and cheap to construct.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
