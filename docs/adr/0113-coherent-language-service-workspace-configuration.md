# ADR-0113: Resolve language-service workspace configuration as one snapshot

- Status: Accepted
- Date: 2026-09-27

## Context

`LanguageServiceWorkspaceController` needs the same workspace conditions for
two related operations: routing documents to language-service clients and
reconciling the clangd lifecycle. Its former interface received separate
callbacks for each boolean and path. A reconciliation could therefore read
different values if settings changed between callbacks, and the controller
had to repeat the same configuration assembly in multiple methods.

The controller also owns one piece of related knowledge that should remain
local: whether the tab set contains a C-family document. That fact is derived
from the live editor session, while workspace configuration comes from the
composition root.

## Decision

Use a named `LanguageServiceWorkspaceController::Configuration` value for:

- language-service enablement;
- C-family enablement;
- the resolved clangd path;
- the active workspace root.

The composition root resolves this value through one callback. The controller
uses one snapshot per operation, applies routing from it, derives the
open-document fact locally, and passes the resulting lifecycle configuration
to `ClangdLifecycleCoordinator`.

Presentation refresh remains a separate callback because it is an output
effect, not part of workspace configuration.

## Alternatives considered

### Keep one callback per field

Rejected because it makes a coherent read an accidental property of callback
ordering and duplicates configuration assembly.

### Move open-document discovery to `MainWindow`

Rejected because the controller already owns the tab-set query needed to
reconcile language-service lifecycle. Moving it outward would make the seam
shallower and leak editor topology into the composition root.

### Store settings inside the controller

Rejected because the controller would become a second source of truth for
persisted settings and runtime resolution.

## Consequences

- Routing and lifecycle reconciliation use the same workspace snapshot.
- A reconciliation resolves configuration once instead of reading correlated
  callbacks independently.
- The interface has one input seam and one presentation output seam.
- Tests can provide a deterministic configuration value and verify read
  coherence without constructing application settings.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testLanguageServiceWorkspaceControllerCoordinatesRouting -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
