# Application shell

## Intent

The application shell should make the current workspace and editor session obvious without competing with the document. The shell is organized around one central work surface and temporary supporting panels.

## Regions

- Activity rail: stable top-level navigation and global actions.
- Workspace panel: file tree, search, Git, and settings views.
- Editor canvas: tabs, breadcrumbs, document, and find/replace.
- Outline panel: optional structural navigation for the current document.
- Bottom panel: diagnostics, compiler output, and references.
- Status strip: language service state, Vim mode, cursor position, indentation, encoding, and language.

## Decisions

- The editor canvas owns horizontal space.
- Side panels can be hidden without changing the current document.
- The outline panel is optional and should not force a permanent three-column layout.
- The bottom panel is task-oriented and should open in response to diagnostics, builds, or references.
- Navigation changes the visible panel; it does not replace the editor session.

## Invariants

- Opening or closing a panel never loses the active document.
- The active navigation mode is visible without relying on hover.
- Workspace identity remains visible when the file tree is hidden.
- The shell works with an empty session through the welcome surface.
