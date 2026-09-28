# ADR-0104: Extract editor appearance policy

- Status: Accepted
- Date: 2026-09-27

## Context

`CodeEditor` combines text input, document synchronization, language-service
requests, cursor decorations, and the visual policy for the editor surface.
Theme changes required the widget to resolve editor-specific colors, mutate
its palette and stylesheet, and schedule all related repaints directly.

That policy was not part of text editing itself, but it was spread across
construction, theme callbacks, gutter painting, current-line highlighting, and
bracket highlighting. Changing the editor visual language therefore required
understanding the input widget and its unrelated document behavior.

## Decision

Introduce `EditorAppearanceController` as the module that resolves the
editor-specific appearance from the active theme and projects the editor
palette and stylesheet. It exposes one `EditorAppearance` value containing the
colors needed by the editor's painting and selection policies, and emits a
change notification after applying a theme.

`CodeEditor` remains the owner of the widget, the line-number surface, and
the text-derived decorations. It consumes the appearance value when painting
the gutter, current line, bracket matches, and LSP selections, and remains
responsible for deciding which surfaces need repainting.

The module is intentionally specific to the editor surface. It does not become
a generic stylesheet builder or take ownership of application-wide theme
application, which remains in `ApplicationThemeController`.

## Alternatives considered

### Keep theme resolution in `CodeEditor`

This keeps color lookups close to their consumers but couples theme policy to
the largest input class. Every visual change would continue to compete with
editing and language-service behavior for locality.

### Move all editor painting into the controller

That would make the controller depend on `QTextDocument`, cursor state, and
line geometry. It would produce a shallow seam because the widget would still
have to expose most of its painting state.

### Reuse `ApplicationThemeController`

The application controller projects the global shell theme onto several
widgets. It does not know editor-specific roles or the editor's repaint
invariants. Reusing it would make the shell controller depend on editor
internals and would blur the two visual scopes.

## Consequences

- Theme resolution and editor palette/stylesheet policy have one locality.
- `CodeEditor` no longer stores theme lookup details or owns the editor
  stylesheet construction.
- Painting remains local to the widget, with a small value interface crossing
  the seam.
- The appearance policy can be tested without constructing a full `CodeEditor`.
- New editor visual roles must be added to `EditorAppearance` deliberately;
  the value is not a replacement for a generic theme dictionary.
