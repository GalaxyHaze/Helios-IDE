---
status: accepted
refinement: ADR-0120 moves asynchronous task-output tracking into
  WorkspaceTaskOutputController
---

# Workspace command controller

The policy for Zith workspace commands lives in
`WorkspaceCommandController`. The controller owns the build/check/run/stop
workflow, interpretation of command results, and the association between
runtime state and the task-output collaborator. `WorkspaceTaskOutputController`
owns buffering of process output received before a run task is announced and
the corresponding `CompilerPanel` projection.

`MainWindow` remains responsible for window composition and supplies callbacks
for workspace lookup, active editor lookup, save-all, diagnostics presentation,
and status messages. State changes are published through the controller's
`stateChanged` signal, which the shell observes for action refresh; action
refresh is not a second callback path. The controller does not own menus, the
bottom dock, settings, or the LSP client process; it uses the existing client
as its command transport.

## Considered options

- Keep command handling in `MainWindow`: rejected because command policy,
  process buffering, progress handling, and result interpretation were spread
  across signal handlers and action slots.
- Put the policy in `CompilerPanel`: rejected because the panel is a visual
  presentation module and should not know workspace roots, LSP commands, or
  editor synchronization.
- Create a generic task framework: rejected because the current variation is
  specifically the Zith workspace-command protocol; a generic framework would
  add an abstraction without a second adapter.

## Consequences

- Build/check/run behavior has one seam and can be tested without constructing
  `MainWindow`.
- Early process output and exits remain associated by task ID in one owner.
- UI composition remains in `MainWindow`, while runtime-task policy has better
  locality.
- The controller currently receives callback dependencies rather than a
  second transport interface; introducing an LSP fake seam is deferred until
  command execution itself needs isolated protocol tests.
