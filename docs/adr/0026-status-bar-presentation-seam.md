# ADR 0026: Isolar a apresentação da barra de estado

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` criava e atualizava diretamente oito labels da barra de estado.
Além da composição dos widgets, o shell conhecia a formatação de contexto,
diagnósticos, posição do cursor, linguagem, modo Vim e o esquema de cores do
estado do LSP. Mensagens temporárias também eram publicadas diretamente em
vários handlers do shell.

Essa concentração tornava uma mudança visual transversal difícil de localizar
e fazia `EditorChromeController` depender de labels concretos para atualizar
posição e linguagem.

## Decision

`StatusBarController` owns the status-bar presentation policy:

- creates the persistent status labels and defines their ordering;
- formats context, diagnostics, cursor position and Vim mode;
- preserves and recolors the LSP state across theme changes;
- owns temporary status messages;
- exposes semantic updates rather than label widgets.

`EditorChromeController` now receives callbacks for editor position and
language instead of `QLabel` pointers. `MainWindow` remains the composition
root and adapts application events to the controller, but no longer owns the
status-bar labels or their formatting.

## Alternatives considered

### Keep labels in `MainWindow`

Would avoid a new module, but every visual status change would continue to
cross the shell and theme code would need to know the status-bar structure.

### Expose labels from a layout module

Would move allocation but preserve the wrong interface: callers would still
depend on widget identity and formatting details rather than status semantics.

### Make each label a separate controller

Would create shallow modules and spread one presentation policy across many
seams. The status bar has a coherent lifecycle and theme contract, so one
controller provides better locality.

## Consequences

Status-bar changes and tests are localized. Editor chrome is now testable with
semantic callbacks and does not require constructing presentation labels.
`MainWindow` still decides when application events occur, while the controller
decides how status is represented.

The controller remains Qt-facing because its responsibility is presentation,
not domain state. It is not intended to become a general application event
bus.

## Verification

`testStatusBarControllerOwnsPresentationState` covers context, LSP, Vim,
diagnostic, cursor-position and language updates. Existing editor chrome
tests cover the callback seam and clearing behavior.
