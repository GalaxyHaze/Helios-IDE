# Tooling feedback

## Intent

Diagnostics, builds, runtime resolution, and language-service progress should be visible without interrupting editing. Feedback is grouped by task and keeps failures actionable.

## Decisions

- Diagnostics are grouped by document and severity.
- Compiler output and runtime progress live in the bottom panel.
- LSP connection state is persistent in the status strip; detailed logs are on demand.
- Success feedback is brief and non-blocking.
- Errors identify the affected document, task, or runtime and expose the next useful action.
- Progress uses one stable surface rather than spawning repeated transient dialogs.

## Invariants

- A diagnostic points to a document location or clearly states why it cannot.
- Progress from an obsolete task cannot overwrite the active task.
- Stopping a runtime task leaves the editor usable and reports the final state.
- Network/runtime failures do not erase the last useful editor state.
