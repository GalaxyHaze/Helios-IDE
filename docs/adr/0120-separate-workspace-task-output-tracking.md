# ADR-0120: Separate workspace task output tracking from command policy

- Status: Accepted
- Date: 2026-09-27

## Context

`WorkspaceCommandController` owned two different policies: deciding whether
Build, Check, Run, and Stop were allowed, and tracking asynchronous process
output around a task announcement. The latter included buffering output and
exits that arrived before `zith.run` returned a `taskId`, projecting progress
into `CompilerPanel`, and clearing task state when the LSP stopped.

That made command policy tests and interfaces carry protocol-event handling that
was not needed to decide or issue a command.

## Decision

Introduce `WorkspaceTaskOutputController` as the module that owns process
output, process exits, progress, early-output buffering, task announcement, and
cleanup after an LSP stop. It connects to the supplied `LspClient` and exposes
the current task identifier plus a semantic state-change callback.

`WorkspaceCommandController` remains responsible for command preconditions,
LSP command requests, command-result interpretation, diagnostics refresh, and
the `WorkspaceCommandState` snapshot. It composes the task-output module and
uses its task identifier when evaluating Stop and publishing state.

The task-output module remains specific to the Zith workspace-command protocol;
no generic process framework is introduced.

## Alternatives considered

### Keep output tracking inside `WorkspaceCommandController`

Rejected because command eligibility, request construction, result parsing, and
asynchronous process projection changed for different reasons and required
different test seams.

### Move task tracking into `CompilerPanel`

Rejected because `CompilerPanel` is a presentation adapter. It should render
task state, not understand LSP process events or early task announcements.

### Create a generic process/task framework

Rejected because there is only one protocol adapter and no second task
implementation that would justify the abstraction.

## Consequences

- `WorkspaceCommandController` has a smaller interface and less event wiring.
- Early output and exits still remain ordered by task identifier.
- The application shell remains unaware of protocol event routing.
- The task-output module can be tested through emitted `LspClient` events and
  `CompilerPanel` projections.
- Future command protocols should not be added to this module without a real
  shared invariant or a second adapter.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testWorkspaceCommandControllerAttachesEarlyRunOutput testWorkspaceCommandControllerOwnsProcessStopCleanup testWorkspaceCommandControllerOwnsClientEventWiring -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
