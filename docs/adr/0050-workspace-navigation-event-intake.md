# ADR 0050: Isolar o event intake de navegação do workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` ligava diretamente sinais do `FileTreePanel`, `WelcomeWidget` e
`GitPanel` a operações do shell. As ligações representavam intenções
relacionadas, mas estavam misturadas com a composição da janela: abrir ficheiro,
abrir numa nova tab, selecionar uma raiz, pedir uma pasta e criar um projeto.

Além de aumentar o conhecimento do composition root sobre cada superfície, a
duplicação tornava fácil esquecer uma origem quando uma nova regra de abertura
fosse introduzida.

## Decision

Criar `WorkspaceNavigationController` como módulo de event intake. Ele adapta:

- `FileTreePanel::fileActivated` e `GitPanel::fileActivated` para `openFile`;
- `FileTreePanel::fileActivatedInNewTab` para `openFileInNewTab`;
- `FileTreePanel::projectRootChanged` e `WelcomeWidget::projectSelected` para
  `selectWorkspaceRoot`;
- `WelcomeWidget::openFolderRequested` para `openFolder`;
- `WelcomeWidget::newProjectRequested` para `newProject`.
- intents `OpenFolder` e `NewProject` vindos do shell para os mesmos
  callbacks, mantendo a origem do pedido independente da superfície.

O controller não decide como validar uma raiz, persistir projetos recentes,
criar tabs ou abrir diálogos. Essas decisões continuam atrás dos callbacks
fornecidos pelo composition root.

## Alternatives considered

### Manter as conexões em `MainWindow`

Rejeitado porque mantém no composition root o conhecimento de todas as
superfícies de entrada e permite que a mesma intenção seja adaptada de forma
inconsistente.

### Colocar a política nos painéis

Rejeitado porque faria um painel conhecer editor sessions, contextos ou
diálogos da aplicação e impediria reutilizar a superfície em outro shell.

### Fundir o controller com `ContextWorkspaceController`

Rejeitado porque navegação é entrada de intenção, enquanto transição de
contexto é a política que reage a uma mudança já aceita. A fusão criaria uma
interface mais larga e misturaria dois ciclos de vida.

## Consequences

`MainWindow` constrói uma única seam para as entradas de navegação e fornece
adapters explícitos para as operações do shell. Os painéis permanecem focados
na apresentação e emissão de sinais; `ContextManager` continua dono das
transições de contexto.

Novas superfícies que apenas emitem intenções de navegação devem ser ligadas
ao controller. Regras de validação, persistência ou restauração não devem ser
adicionadas a ele.

## Verification

`testWorkspaceNavigationControllerRoutesWorkspaceIntents` emite os sinais das
três superfícies, envia intents `OpenFolder`/`NewProject` e verifica as cinco
intenções através dos callbacks. Build CMake, CTest e `git diff --check` passam
após a alteração.
