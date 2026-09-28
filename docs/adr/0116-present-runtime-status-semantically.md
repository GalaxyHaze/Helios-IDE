# ADR-0116: Present runtime status from semantic states

- Status: Accepted
- Date: 2026-09-27

## Context

The LSP runtime controllers need to report transitions such as disabled,
starting, warming, connected, and error. Encoding those transitions as status
bar strings and theme colors in the lifecycle and event controllers coupled
runtime policy to one visual surface and made future presentation changes
likely to spread across several modules.

## Decision

`LspRuntimePresentationController` accepts a `RuntimeStatus` value and owns the
translation from that semantic state to status-bar text and theme color.
`LspRuntimeController` and `LspRuntimeEventController` publish only semantic
runtime states; they do not choose presentation strings or colors. Frontend
JSON states are also translated at the presentation seam.

## Alternatives considered

### Keep strings and colors in the runtime controllers

Rejected because it preserves a hidden dependency from lifecycle policy to the
current status-bar vocabulary and theme implementation.

### Introduce a general event bus

Rejected because the problem is a narrow presentation seam, not a need for
global event distribution. A typed enum keeps ownership and data flow local.

## Consequences

- Runtime lifecycle policy can change independently of status-bar wording and
  theme roles.
- Presentation tests can exercise the semantic-to-visual mapping without
  constructing the runtime process.
- Adding a new runtime state requires updating one typed mapping and its tests.
- The presenter remains responsible for projection, not runtime lifecycle,
  process control, or restart policy.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testLspRuntimePresentationProjectsFrontendEvents -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
