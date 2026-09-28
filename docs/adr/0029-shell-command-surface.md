# ADR 0029: Separar a superfície de comandos do shell

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` criava menus, atalhos de menu, traduções, ações de build/run e
ações de navegação. O mesmo arquivo também calculava enabled/tooltips a partir
do estado do LSP, editor e compiler panel. Isso fazia o composition root
conhecer cada `QAction` e espalhava a política visual dos comandos.

Expor cada action através de getters reduziria linhas, mas criaria uma
interface rasa: o consumidor continuaria dependente da estrutura concreta do
menu.

## Decision

`ShellCommandSurface` owns the menu hierarchy, menu action widgets, and global
keyboard shortcuts. Its external seam is:

- one `commandRequested(Command)` signal for user intent;
- installation of the shared global shortcut vocabulary;
- `WorkspaceActionState` for the calculated availability and tooltips of
  build/check/format/run/stop;
- semantic setters for restart-LSP, outline and bottom-panel checked state;
- one translation operation.

`MainWindow` remains the application-shell adapter for commands that cross
application boundaries and computes workspace/editor state where necessary. It
delegates domain-owned command subsets to the corresponding controllers. It
does not own individual menus or `QAction*` members.

## Alternatives considered

### Expose every action to `MainWindow`

Would preserve the old coupling under a new class and create a large,
widget-shaped interface with little depth.

### Let the surface execute application behavior

Would make menu construction depend on runtime, documents, dialogs and
workspace policy, turning a presentation module into a second composition
root.

### Use one signal per action

Would be explicit but would multiply the interface without adding semantics.
The command enum keeps one stable intent seam while retaining readable command
names.

## Consequences

Menu structure, keyboard shortcuts, translations and action presentation are
localized. Tests can exercise command emission and action-state invariants
without constructing a `MainWindow`.

The enum must be extended when a new shell command is introduced. The shell
coordinates application state, while domain controllers own the behavior
behind commands that belong to their policies; `MainWindow` retains only
cross-domain composition commands.

## Verification

`testShellCommandSurfaceEmitsIntentAndAppliesActionState` verifies intent
emission and enabled/tooltip projection. The build and complete test suite pass
after the extraction.
