# ADR 0015: Isolar ações LSP iniciadas pelo editor

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow::connectEditorSignals` continha a política de duas ações iniciadas
por um editor: antes de rename, todas as tabs tinham de enviar as alterações
pendentes; code actions tinham de usar o cliente associado ao editor e os seus
diagnostics atuais. A janela também conhecia diretamente os detalhes dos
requests LSP.

Essa regra era pequena em linhas, mas importante em ordenação: enviar rename
antes do flush podia fazer o servidor trabalhar sobre uma versão antiga do
workspace.

## Decision

`EditorLspActionController` recebe o `QTabWidget`, liga os sinais de cada
editor e expõe operações que devolvem se o request foi despachado:

- `requestRename` faz flush de todas as tabs antes de verificar e usar o
  cliente LSP do editor;
- `requestCodeActions` envia o range e os diagnostics do editor apenas quando
  o cliente está pronto;
- clientes ausentes ou não prontos são rejeitados sem efeitos externos.

`MainWindow` continua a compor a janela e a tratar os resultados (workspace
edits, menus e mensagens), mas não implementa mais a preparação dos requests.

## Alternatives considered

### Manter a lógica em `MainWindow`

Tem menos classes, mas deixa a ordenação crítica escondida numa janela que já
coordena menus, runtime, tabs e apresentação.

### Colocar as ações no `CodeEditor`

O editor não possui a coleção de tabs e não deve decidir quando sincronizar
outros documentos. Isso violaria a separação entre documento individual e
workspace.

### Criar um controlador geral de interações

Misturaria Vim, zoom, navegação, chrome e LSP numa interface larga. A seam
atual cobre apenas a política LSP que exige coordenação entre documentos.

## Consequences

O ordering constraint do rename fica local e testável. Novos tipos de ação
LSP podem ser adicionados sem aumentar o método de ligação geral da janela.
O módulo depende deliberadamente de widgets reais porque a sua regra atravessa
o conjunto de tabs abertas.

## Verification

`testEditorLspActionsRejectUnavailableClients` cobre a recusa segura quando o
cliente não está pronto; o comportamento de flush e despacho é exercido pelos
signals ligados em `MainWindow`.
