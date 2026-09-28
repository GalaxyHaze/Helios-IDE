# ADR 0020: Separar routing e apresentação de code actions LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` recebia resultados de code actions e, no mesmo lambda, criava o
menu, decidia se uma ação era um `WorkspaceEdit`, verificava suporte a
`executeCommand` e ligava os callbacks de mutação e execução. Isso fazia o
shell conhecer detalhes do formato de ações LSP e tornava a apresentação
difícil de testar sem abrir a janela real.

## Decision

`LspCodeActionRouter` recebe callbacks para aplicar um workspace edit,
executar um comando e apresentar um `QMenu`. O módulo:

- ignora respostas vazias;
- cria uma entrada por ação usando o título recebido;
- liga ações com `edit` ao callback de `WorkspaceEdit`;
- liga ações com `command` apenas quando o cliente anuncia
  `executeCommandProvider`;
- desabilita ações que não têm uma forma executável;
- apresenta o menu através de um adapter síncrono, permitindo testes sem
  bloquear em `QMenu::exec()`.

`WorkspaceEditApplier` continua sendo o seam responsável pela mutação; o
router não interpreta ranges nem conhece tabs ou ficheiros.

## Alternatives considered

### Manter a composição no `MainWindow`

Reduz o número de ficheiros, mas mistura protocolo, apresentação e mutação no
shell e duplica conhecimento sempre que outro cliente LSP é ligado.

### Colocar code actions em `WorkspaceEditApplier`

Isso faria um módulo de mutação conhecer menus e comandos remotos. A
aplicação de edits e a escolha da ação são decisões diferentes.

### Expor somente `QJsonArray` para o shell

Seria uma extração superficial: o shell continuaria decidindo títulos,
habilitação e callbacks. O router esconde essa política atrás de `attach`.

## Consequences

O shell só compõe clientes e callbacks de infraestrutura. A apresentação
continua substituível por um adapter síncrono, enquanto a aplicação de edits
permanece centralizada no `WorkspaceEditApplier`.

## Verification

`testLspCodeActionRouterBuildsAndExecutesEditActions` verifica a composição
do menu e a execução de uma ação com workspace edit através da interface do
router.
