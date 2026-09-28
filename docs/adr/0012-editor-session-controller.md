---
status: accepted
---

# Editor session controller

The lifecycle of open editor documents lives in `EditorSessionController`.
The controller creates and initializes editor tabs, opens local files,
updates tab metadata, rebinds untitled editors to named files, synchronizes
documents with language services, saves documents (including the untitled-to-
named transition), closes documents, performs LSP save-all, and captures or
restores the `EditorSessionState`.

`MainWindow` remains responsible for composing the tab layout and for the
window-specific policies supplied through callbacks: editor preferences,
editor signal wiring, chrome updates, central-widget state, clangd lifecycle,
LSP routing refresh, and log presentation. The session controller requests a
routing refresh before document synchronization; it does not decide which
language service should handle a path. For an interactive Save As operation,
the window chooses the destination path and presents feedback; the controller
owns the file write, document rebind, modified-state transition, and
post-save language-service synchronization. The controller does not own the
application shell or context navigation.

## Considered options

- Keep tab and document lifecycle in `MainWindow`: rejected because opening,
  saving, closing, save-all, and context restoration crossed the same
  editor/LSP invariants in several unrelated window handlers.
- Create a generic tab manager: rejected because the useful policy is about
  documents and language-service synchronization, not arbitrary tabs.
- Move all editor signal handling into the controller: rejected because
  navigation, Vim commands, rename, and code actions are interaction policy
  that still belongs to the window host and has different dependencies.

## Consequences

- Session invariants such as “close the LSP document before deleting the
  editor”, “write before rebinding an untitled editor”, and “restore only
  named files” have one implementation.
- Context restoration can be tested without constructing `MainWindow`.
- The controller has a focused public interface, while host-specific behavior
  crosses explicit callbacks.
- The current implementation still uses a concrete `QTabWidget` and
  `CodeEditor`; a lower-level document model is deferred until a second UI
  surface needs the same session policy.
