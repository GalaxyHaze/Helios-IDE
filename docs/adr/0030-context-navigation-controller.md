# ADR 0030: Isolar a política de navegação entre contextos

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` criava os atalhos `Alt+Left` e `Alt+Right` e também conhecia a
política de histórico de contextos. Em particular, `Alt+Right` tinha dois
significados: avançar num contexto já existente ou, quando o cursor estava no
fim e havia uma raiz ativa, pedir uma nova pasta e criar um contexto.

Essa regra não pertence ao armazenamento de contextos nem à restauração de
tabs. Mantê-la no composition root fazia a janela conhecer detalhes da
navegação e tornava difícil testar o caso de cancelamento sem construir toda a
IDE.

## Decision

`ContextNavigationController` owns the context-navigation policy and installs
the two keyboard shortcuts. Its interface receives:

- o `ContextManager` que possui o histórico;
- uma callback para persistir o estado do contexto ativo;
- uma callback para pedir uma nova raiz de workspace.

Before any actual movement or context creation, the controller asks the host
to persist the active state. A cancelled root request has no side effect.
`ContextManager` continues to own context storage, indices, and
`contextChanged` notifications. `MainWindow` remains the adapter that supplies
the persistence and file-dialog callbacks.

## Alternatives considered

### Keep the shortcuts in `MainWindow`

Would avoid a class, but keep history policy coupled to workspace composition,
dialogs, LSP setup, and panel wiring. The rule would remain difficult to test
in isolation.

### Put the policy in `ContextManager`

Would make a domain storage module depend on keyboard shortcuts, widgets, and
the file-dialog interaction required to create a context.

### Expose navigation actions from `ContextManager`

Would move only the method calls and leave the significant branching policy in
the shell. It would be a shallow interface with no useful new seam.

## Consequences

Context navigation and cancellation behavior have a focused test surface.
`MainWindow` loses shortcut construction and history branching, while still
deciding how context state is serialized and how a workspace root is selected.
Adding another navigation input can reuse the controller without changing
context storage.

## Verification

`testContextNavigationControllerPreservesHistoryPolicy` verifies shortcut
installation, persistence before movement, creation at the end of history, and
the no-op behavior when root selection is cancelled. The complete build and
test suite must pass after the extraction.
