# ADR-0105: Extract editor context-menu policy

- Status: Accepted
- Date: 2026-09-27

## Context

`CodeEditor::contextMenuEvent` was responsible for constructing every
language-aware action, checking provider availability, opening the rename
prompt, copying the symbol under the cursor, translating labels, and styling
the menu. This made a transient presentation policy part of the same class
that owns text input, document synchronization, decorations, completion, and
Vim behavior.

The menu also had a meaningful invariant: an action must be enabled only when
the current language-service session advertises the corresponding feature.
That invariant was easy to miss when changing unrelated editor input code.

## Decision

Introduce `EditorContextMenuController` with a small `createMenu`/`show`
interface. The module owns the menu's action inventory, translation lookup,
feature availability projection, rename prompt, symbol copy action, and menu
stylesheet. `createMenu` returns an unshown menu so the policy can be tested
through its real action surface; `show` is the runtime convenience that
creates and executes it.

`CodeEditor` remains the owner of the context-menu event and supplies the
editor and its `EditorLanguageFeatureController` to the module. The language
feature controller remains the owner of request preparation and document
version flushing; the context-menu module only chooses and presents the
intent.

The menu is created for each invocation because its enabled state and labels
depend on the current language-service session and locale. The controller
does not become a general command registry or a second language-service
router.

## Alternatives considered

### Keep construction in `CodeEditor`

This keeps the code near the event override but couples menu policy to every
other editor concern. It also makes menu availability changes harder to test
without understanding the full widget.

### Put menu actions in `EditorLanguageFeatureController`

That controller owns feature availability and request preparation, not
presentation or user prompts. Moving the menu there would make a language
service module depend on translation, theme, and clipboard policy.

### Introduce a generic command registry

The menu has a small, editor-specific set of actions whose availability is
document and session dependent. A registry would add indirection without
creating a second adapter or a genuine variation point.

## Consequences

- `CodeEditor` exposes fewer private action wrappers and delegates one policy.
- Menu availability, translation, styling, and prompts have one locality.
- The language feature controller remains independent of menu presentation.
- Menu composition can be verified without opening a modal popup.
- Future editor actions should be added to this controller only when they are
  genuinely context-menu actions; global commands belong to the shell surface.
