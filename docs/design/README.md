# Helios IDE Design

This directory is the visual design source of truth for future IDE UI work.

Each design aspect has two files:

- `.md`: decisions, intent, invariants, and review questions.
- `.html`: a lightweight visual preview that can be opened directly in a browser.

`README.md` is only the index and session guide; it is not a design aspect and
therefore has no matching preview file.

Future design sessions should read the relevant pair before changing layouts, colors, typography, navigation, or feedback surfaces. A visual change should update both files when it changes the intended design rather than only the implementation detail.

## Aspects

- [Visual language](./visual-language.md) and [preview](./visual-language.html)
- [Application shell](./application-shell.md) and [preview](./application-shell.html)
- [Editor workspace](./editor-workspace.md) and [preview](./editor-workspace.html)
- [Editor appearance](./editor-appearance.md) and [preview](./editor-appearance.html)
- [Editor context menu](./editor-context-menu.md) and [preview](./editor-context-menu.html)
- [Syntax and language identity](./syntax-and-language.md) and [preview](./syntax-and-language.html)
- [Tooling feedback](./tooling-feedback.md) and [preview](./tooling-feedback.html)
- [Source control](./source-control.md) and [preview](./source-control.html)

## Future-session checklist

1. Read the matching `.md` before proposing a UI change.
2. Open the matching `.html` before changing spacing, hierarchy, or emphasis.
3. Preserve the stated hierarchy and interaction invariants.
4. Update both files when the design decision changes.
5. Record a durable trade-off in `docs/adr/` only when it is hard to reverse, surprising, and based on real alternatives.
