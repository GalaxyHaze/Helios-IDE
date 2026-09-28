# ADR-0117: Let the editor session own editor preferences

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` supplied editor font, word-wrap, and Vim settings to
`EditorSessionController` through an unlabelled callback. That forced the
composition root to own the policy for applying preferences whenever a tab was
created or appearance changed, and it made the session interface depend on a
callback that carried no semantic contract.

## Decision

`EditorSessionController` accepts an `EditorPreferences` value containing the
editor font, word-wrap mode, and Vim enablement. It applies the value when
creating a tab and can reapply it to all existing tabs. `MainWindow` remains
the source of application settings, but only composes and supplies the value;
it no longer applies editor preferences editor-by-editor. `TomlSettingsStore`
publishes `editorPreferencesChanged()` when word-wrap or Vim enablement
changes, so the composition root can recompose the value without maintaining a
stale copy of individual settings.

## Alternatives considered

### Keep an `applyEditorPreferences` callback

Rejected because it hides the policy behind an untyped callback and makes the
session controller unable to guarantee that new and existing tabs receive the
same configuration.

### Add a dedicated editor-preferences adapter

Rejected because there is one stable value input and no second adapter or
varying implementation. A value object gives the seam depth without adding a
pass-through module.

## Consequences

- New and existing tabs share one explicit preference shape.
- Appearance changes are reapplied through the session module rather than a
  loop in `MainWindow`.
- Word-wrap and Vim changes use one semantic settings event and cannot leave
  existing tabs on a stale copy held by the composition root.
- Tests can verify preference application through the session interface.
- Future editor preferences should be added to the value only when the session
  owns their application policy.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testEditorSessionControllerAppliesEditorPreferences -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
