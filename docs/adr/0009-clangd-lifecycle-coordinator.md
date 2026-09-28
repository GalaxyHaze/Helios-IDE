---
status: accepted
---

# Clangd lifecycle coordinator

The policy for starting, stopping, reusing, and reporting the clangd process
lives in `ClangdLifecycleCoordinator`. The coordinator receives explicit
configuration describing global LSP enablement, C-family enablement, open
documents, executable path, and workspace root. It owns the operational state
and delegates process actions to the existing `LspClient`.

`MainWindow` remains responsible for discovering settings, counting open
editors, translating operational state for the UI, and reconnecting open
documents after `LspClient::initialized`. The coordinator does not own widgets,
settings storage, translations, or document presentation.

## Considered options

- Keep lifecycle decisions in `MainWindow`: rejected because every settings,
  tab, and error path had to repeat process policy beside UI code.
- Add a generic process supervisor abstraction: rejected because only clangd
  currently needs this policy and a generic interface would hide the useful
  configuration seam.
- Move document synchronization into the lifecycle coordinator: rejected
  because process lifecycle and per-editor document binding have different
  ownership and timing rules; document synchronization remains in
  `LspDocumentCoordinator`.

## Consequences

- Lifecycle transitions can be tested without constructing the main window.
- A clangd path change, workspace-root change, enablement change, or
  document-count change follows one reconciliation path.
- UI wording remains localized at the presentation boundary.
- The coordinator currently depends on the concrete `LspClient`; introducing a
  fake client seam is deferred until lifecycle behavior needs broader isolated
  testing.
