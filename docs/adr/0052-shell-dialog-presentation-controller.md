# ADR 0052: Isolar a apresentação dos diálogos do shell

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` acumulava a criação lazy e a política de toggle de Preferences e
Vim Help, além de repetir a mesma sequência de `show`, `raise` e
`activateWindow` para Shortcuts e LSP Manager. A mesma janela também precisava
entregar o `LspManagerDialog` aos controllers de runtime e de apresentação,
misturando lifecycle de diálogo com projeção de estado LSP.

## Decision

Criar `ShellDialogController` para:

- criar Preferences, Shortcuts e Vim Help apenas quando solicitados;
- aplicar uma política única de toggle, raise e activate;
- abrir/fechar o LSP Manager e pedir refresh de clangd somente ao abrir;
- apresentar o diálogo Getting Started de forma modal;
- reconhecer as intenções de diálogo através de
  `handleShellCommand(ShellCommand)`, sem expor esse dispatch ao composition
  root;
- expor o `LspManagerDialog` como destino de apresentação para os controllers
  de runtime.

O controller recebe o parent dos diálogos e o `LspManagerDialog` já composto.
Não possui estado LSP, não persiste preferências e não interpreta o conteúdo
interno dos diálogos. A seam usa `QDialog*` para as janelas auxiliares, não
`QWidget*`, tornando explícita a invariável de apresentação.

## Alternatives considered

### Manter os toggles em `MainWindow`

Rejeitado porque repetia lifecycle visual e obrigava o composition root a
conhecer detalhes de cada diálogo.

### Fazer o LSP Manager possuir o próprio refresh

Rejeitado porque o diálogo não deve conhecer lifecycle de clangd/runtime; o
controller apenas solicita refresh através de um adapter.

### Esconder completamente o LSP Manager atrás do controller

Rejeitado nesta etapa porque `LspRuntimeController` e
`LspRuntimePresentationController` precisam de um destino explícito para
projetar estado e receber eventos. O accessor é limitado a esse papel e não
devolve a política de abertura.

### Criar um controller genérico para qualquer `QWidget`

Rejeitado porque perderia a profundidade da seam e permitiria misturar painéis,
dialogs e superfícies que têm ciclos de vida diferentes.

## Consequences

`MainWindow` deixa de possuir quatro políticas de diálogo e conserva apenas a
composição dos módulos. A criação lazy fica localizada, e futuras mudanças no
comportamento de abertura/fecho devem ser feitas no controller. Alterações
visuais internas dos diálogos continuam pertencendo aos próprios módulos e,
quando mudarem a linguagem visual da IDE, devem atualizar o par `.md` + `.html`
relevante em `docs/design/`.

## Verification

`testShellDialogControllerOwnsDialogPresentationPolicy` verifica criação lazy,
toggle, visibilidade, refresh único do LSP Manager e o dispatch semântico de
comandos de diálogo. Build CMake, CTest e `git diff --check` passam após a
extração.
