# ADR-0102: Extract LSP session lifecycle

- Status: Accepted
- Date: 2026-09-27

## Context

`LspClient` exposed the editor-facing language-service interface, but also
owned several independent lifecycle rules: selecting startup options,
initializing a process, deciding readiness, performing the LSP shutdown
handshake, escalating from terminate to kill, clearing session state, and
restarting after a replacement request arrived while a process was running.

Those rules were difficult to test without launching a process and made
changes to shutdown behavior risky because protocol dispatch, document
synchronization, feature routing, and process lifecycle shared one
implementation.

## Decision

Introduce `LspSessionLifecycle` as the module responsible for the language
service process session state. Its interface accepts explicit callbacks for
transport actions and session-owned data reset, and exposes lifecycle signals
for initialization, errors, and process completion.

`LspClient` remains the compatibility façade used by the editor. It retains
the public request and signal surface, while delegating startup, readiness,
shutdown escalation, and pending restart behavior to the lifecycle module.

The lifecycle module does not own protocol transport, document versions,
request tracking, or feature selection. Those remain separate modules and
cross the seam through callbacks.

## Alternatives considered

### Keep lifecycle code in `LspClient`

This preserves locality only superficially. Every lifecycle change would
continue to require understanding the complete protocol façade, and direct
tests would need a real process or private state access.

### Create a generic process supervisor

A generic supervisor would hide the LSP-specific shutdown handshake and
initialization ordering behind a leaky abstraction. The current seam is
deliberately specific to an LSP session.

### Move only the shutdown timer

That would leave startup, restart intent, readiness, and reset invariants
scattered across two modules. The selected module owns the complete lifecycle
transition instead of layering another shallow adapter.

## Consequences

- Session transitions can be tested with in-memory callbacks.
- `LspClient` is smaller and its compatibility interface remains stable.
- Transport and protocol modules remain independently replaceable.
- Callback wiring is an internal composition-root concern; callers do not
  depend on `LspSessionLifecycle` unless they are testing or evolving the
  session seam.
- Unexpected process-exit diagnostics are assembled by `LspClient` from its
  stderr buffer before the lifecycle resets session data.
