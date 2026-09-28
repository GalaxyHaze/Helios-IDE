---
status: accepted
---

# Editor syntax controller

`EditorSyntaxController` owns the mapping between a document language and its
`QSyntaxHighlighter`, including replacement and cleanup when an editor changes
language. `MainWindow` remains responsible for determining the language from
the path and delegates highlighter lifecycle to this module.

The controller preserves the current behavior: C-family files use
`CHighlighter`, Zith and plain-text files use `SyntaxHighlighter`, and changing
between those identities replaces the existing highlighter before attaching
the new one.

## Considered options

- Keep the highlighter map in `MainWindow`: rejected because ownership and
  language transition rules were unrelated to window composition.
- Create one highlighter per editor and never replace it: rejected because
  untitled files can acquire a language when saved and C-family syntax has a
  different rule set.
- Put language classification inside the controller: rejected because
  `LanguageIdentity` is the existing source of truth and the controller should
  receive an explicit language.

## Consequences

- Highlighter ownership and cleanup are testable without constructing the main
  window.
- Language transitions have one implementation and cannot silently leave the
  old highlighter attached.
- Visual syntax behavior remains unchanged; future syntax changes should update
  the corresponding design pair in `docs/design/`.
