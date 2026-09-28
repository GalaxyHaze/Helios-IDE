# ADR-0106: Centralize LSP log projection

- Status: Accepted
- Date: 2026-09-27

## Context

LSP messages were projected in several unrelated callers. `MainWindow`
duplicated the same settings-panel and manager-dialog policy for editor-session
logs and runtime-event logs, while `LspRuntimePresentationController` repeated
the policy for frontend status and metrics.

The duplication was not merely cosmetic. The settings panel is conditional on
the global LSP enablement, while the LSP manager remains useful when LSP is
disabled or is failing to start. A caller that copied only one half of this
rule could make diagnostics disappear from one surface or expose disabled
service logs in another.

## Decision

Introduce `LspLogPresenter` as the single module responsible for projecting an
LSP message. Its interface is one operation, `append`, and its implementation
owns:

- the conditional projection into `SettingsPanel`;
- the unconditional projection into `LspManagerDialog`;
- the enablement query needed to evaluate that policy.

`EditorSessionController`, `LspRuntimeEventController`, and
`LspRuntimePresentationController` depend on this module instead of accepting
callbacks that reproduce the projection policy. `MainWindow` composes one
presenter and passes it to those modules. The presenter is not a general
event bus and does not format protocol messages or own runtime lifecycle.

## Alternatives considered

### Keep callbacks in `MainWindow`

Rejected because each new producer would need to duplicate the same invariant
and the composition root would accumulate presentation policy.

### Let every panel subscribe directly to LSP clients

Rejected because it spreads routing, enablement, and ordering decisions across
widgets. It also makes the manager and settings panel disagree more easily
about which messages they should show.

### Introduce a generic message-bus module

Rejected because there is one concrete projection policy and no second adapter
or transport variation. A bus would be a shallow abstraction with a larger
interface than the behaviour it hides.

## Consequences

- LSP log visibility has one locality and one test seam.
- Runtime and editor-session controllers have smaller callback interfaces.
- `MainWindow` remains a composition root rather than a log-policy owner.
- Adding another log surface requires changing one presenter and its tests.
- The presenter intentionally depends on concrete shell panels; it is a
  shell-facing adapter, not a protocol-domain abstraction.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
