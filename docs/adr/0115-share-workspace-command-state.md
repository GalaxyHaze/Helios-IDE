# ADR-0115: Share one workspace command state snapshot

- Status: Accepted
- Date: 2026-09-27

## Context

Workspace command execution and command-surface availability need the same
facts: whether the language service can execute commands, whether the active
editor is a Zith document, whether it has a persistent file, whether a
workspace root exists, whether a task is running, and whether formatting is
supported.

The previous interface made `WorkspaceCommandAvailabilityController`
reconstruct those facts through several callbacks while also asking
`WorkspaceCommandController` for command-specific predicates. That duplicated
policy and allowed the presentation projection to observe a different
combination of editor, root, and task state from the execution path.

## Decision

Introduce `WorkspaceCommandState` as the semantic snapshot produced by
`WorkspaceCommandController::state()`. The execution controller uses the
snapshot for its predicates, while the availability controller receives the
snapshot explicitly through `refresh(const WorkspaceCommandState&)` and only
projects it into `WorkspaceCommandAvailability`. State transitions publish
the snapshot as the payload of `stateChanged`.

The snapshot contains facts, including the currently tracked task identifier,
not labels, tooltips, menu actions, or compiler side effects.
`WorkspaceCommandAvailability` remains the presentation value owned by the
command-surface adapter.

## Alternatives considered

### Keep independent callbacks in the availability controller

Rejected because it duplicated execution policy and made coherent reads
accidental.

### Return presentation availability directly from the execution controller

Rejected because command execution should not own menu labels and tooltip
wording.

### Make the command surface query the execution controller directly

Rejected because it would couple presentation to the full execution module
instead of crossing a small value-object seam.

## Consequences

- Execution and presentation derive command availability from one snapshot
  shape.
- Build and run requests use the root captured by that snapshot rather than
  rereading workspace state after the availability decision.
- Stop requests use the task identifier captured by that snapshot rather than
  rereading the compiler panel after deciding that a task is running.
- The availability adapter has one explicit value input instead of a callback
  that reads back through the execution module.
- Formatting capability remains represented without adding presentation
  policy to the execution module.
- Future command predicates can be added to the state value and tested once.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testWorkspaceCommandAvailabilityTracksEditorAndTaskState -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
