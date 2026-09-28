# ADR-0108: Extract editor decoration composition

- Status: Accepted
- Date: 2026-09-27

## Context

`CodeEditor` was both the text input surface and the composition point for
several unrelated `QTextEdit::ExtraSelection` sources: diagnostics, bracket
matches, LSP document highlights, find results, and the current-line marker.
Each source had its own state and refresh path, but the widget had to know how
to combine them before every repaint.

That arrangement made visual changes compete with document input and language
feature behavior for locality. It also made it easy for a new selection source
to replace an existing source accidentally instead of participating in one
composition policy.

## Decision

Introduce `EditorDecorationController` as the editor-local module that owns
decoration state and projects it into one `setExtraSelections` call. Its
interface accepts diagnostics, LSP highlight ranges, and find selections, and
provides explicit refresh operations for cursor-dependent decorations and
appearance changes.

The controller owns:

- projection of diagnostics through `EditorDiagnosticHighlighter`;
- bracket matching through `BracketMatcher`;
- conversion of LSP ranges into text selections;
- preservation of find selections;
- current-line selection policy;
- ordering and composition of all selection sources.

`CodeEditor` remains the owner of the document, editor widget, public
diagnostic state, and event wiring. It creates the controller and forwards
editor events, but does not decide how individual decoration sources are
combined. `EditorAppearanceController` remains the source of editor visual
roles; the decoration controller consumes that value and does not resolve
themes.

## Alternatives considered

### Keep all selection composition in `CodeEditor`

Rejected because the widget would continue to own independent visual policies
that are not part of text input. Every new decoration source would increase
the coupling between cursor events, LSP callbacks, search presentation, and
theme changes.

### Create one controller per decoration source

Rejected because the difficult invariant is composition, not the individual
projection algorithms. Splitting diagnostics, brackets, and current-line
selection into separate Qt objects would leave ordering and replacement policy
in the widget.

### Make the controller a generic application decoration service

Rejected because the module depends on one `QPlainTextEdit`, one document, and
editor-local appearance roles. A broader service would reduce depth and make
ownership of text selections ambiguous.

## Consequences

- All editor decoration sources have one composition locality.
- `CodeEditor` has a smaller visual policy surface and remains a composition
  root for editor-specific modules.
- Selection ordering is explicit and can be changed without editing input or
  language-service code.
- The controller can be tested with a text widget without constructing the
  full main window.
- New decoration sources must enter through this controller rather than
  calling `setExtraSelections` directly.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
