# Syntax and language identity

## Intent

Syntax highlighting should reinforce the document's language identity without
changing the document model or adding visual noise to the editor surface.

## Decisions

- Zith documents use the Zith syntax palette and rules.
- C-family documents use the C-family syntax palette and rules.
- Plain-text documents use the neutral syntax highlighter path.
- The language indicator in the status strip and the syntax rules must derive
  from the same language identity.
- Saving an untitled document can change its language and therefore replaces
  the syntax rules for that editor.

## Invariants

- Changing language never loses document text, cursor state, or tab identity.
- A stale highlighter is detached before a new language highlighter is
  attached.
- Syntax colors remain subordinate to selection, diagnostics, and the active
  line.
- A future language addition must define both its language identity and its
  syntax treatment before it is wired into the editor.

## Review questions

- Does the new syntax remain legible in every supported theme?
- Is the status-strip language label consistent with the highlighter?
- Does a language transition preserve diagnostics and document synchronization?
