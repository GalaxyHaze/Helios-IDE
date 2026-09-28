# Editor workspace

## Intent

The editor workspace is the primary product surface. It should preserve orientation across files while making the current document, language, and editing state immediately legible.

## Decisions

- Tabs represent the open documents in the current editor session.
- The active tab is visually dominant; inactive tabs remain recognizable but quiet.
- Breadcrumbs provide path orientation without replacing the file tree.
- Find/replace is attached to the editor canvas so it does not consume a permanent side panel.
- The editor status strip reports local editing state and language-service state separately.
- Vim mode changes input behavior but does not change the document identity or tab model.

## Invariants

- Switching tabs preserves each document's text, modified state, diagnostics, and language identity.
- A language change caused by saving an untitled file re-evaluates highlighting and language-service routing.
- Search selection and diagnostics remain anchored to document coordinates.
- Editor actions should be available from keyboard paths as well as visible controls.
