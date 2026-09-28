# Editor context menu

## Intent

The context menu is a compact command surface for actions that are meaningful
at the current symbol and document location. It should expose language-aware
operations without competing with the permanent shell menus.

## Decisions

- Navigation actions come first because they preserve the user's current
  document context.
- Symbol mutation actions, such as rename and code actions, are grouped after
  navigation and separated visually.
- Formatting is grouped as a document operation rather than a navigation
  action.
- Copying the symbol under the cursor remains available independently of LSP
  readiness.
- Language-service actions are visibly disabled, not silently removed, when
  the active service cannot provide them.
- The menu uses the active application theme but remains visually subordinate
  to the editor canvas.

## Invariants

- Availability is evaluated for the current editor and current language
  service at the moment the menu opens.
- Invoking an action does not change the active document or tab implicitly.
- Rename and code actions use the editor's current document state, including
  pending local changes.
- The context menu is transient; it must not become a second global command
  surface.

## Review questions

- Is this action specific to the symbol or document under the cursor?
- Should an unavailable action remain visible with an explanation?
- Does adding the action make the menu harder to scan than the shell menu?
- Does the action belong in the editor menu, the global shell, or both?
