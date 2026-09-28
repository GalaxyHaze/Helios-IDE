# ADR 0033: Separar o vocabulário de intenção da superfície de comandos

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` recebia `ShellCommandSurface::Command`, embora a janela não
precisasse conhecer menus ou atalhos. O enum de intenção estava aninhado na
classe que o emitia, o que fazia a apresentação visual ser a dona do
vocabulário usado pelo restante do shell.

Isso também tornava mais difícil introduzir outra origem de comandos, como
paleta de comandos, automação ou testes de integração, sem depender da
superfície de menus.

## Decision

`ShellCommand` is a standalone intent vocabulary. `ShellCommandSurface` emits
that type through one `commandRequested` signal and remains responsible for
menus, shortcuts, translations, and action state. `MainWindow` consumes
`ShellCommand` directly and no longer includes the surface header in its
public interface; domain controllers consume the same vocabulary for their
own command subsets.

The existing `ShellCommandSurface::Command` name remains a type alias during
the migration so current surface-local call sites do not need a broad,
behaviorless rewrite.

## Alternatives considered

### Keep the enum nested in `ShellCommandSurface`

Would keep the shell coordinator coupled to a presentation class and make
other command sources depend on menus.

### Define one signal per command

Would duplicate the command vocabulary in the surface interface and enlarge
the number of seams without adding domain meaning.

### Use strings as command identifiers

Would remove the C++ dependency at the cost of compile-time exhaustiveness,
renames, and switch coverage in the shell coordinator.

## Consequences

Command intent has an independent seam and can be emitted by future surfaces
without making them own the menu implementation. The enum remains closed and
requires an explicit update when a new command is introduced. The compatibility
alias can be removed after external consumers have migrated.

## Verification

The application and test targets build with Qt MOC, the existing
`testShellCommandSurfaceEmitsIntentAndAppliesActionState` continues to pass,
and `MainWindow.h` depends on `ShellCommand.h` rather than
`ShellCommandSurface.h`.
