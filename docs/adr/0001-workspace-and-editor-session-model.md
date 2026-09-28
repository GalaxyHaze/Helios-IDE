---
status: accepted
---

# Workspace and editor session model

Helios models navigation as multiple contexts, where each context contains a
workspace root and an `EditorSessionState` value object containing open file
paths and an active tab index; the active context is materialized as the
current editor session. This keeps restorable session data separate from live
Qt widget state and allows users to move between workspace roots without
creating a second application window for each one.

## Consequences

- Context restoration is path-based and must reopen documents before selecting the active tab.
- Unsaved editor contents are not part of the persisted context state.
- Session data is passed as one value object rather than parallel file-list
  and tab-index arguments.
- Workspace navigation, tab lifecycle, and context restoration are related decisions and should remain consistent when the editor is refactored.
