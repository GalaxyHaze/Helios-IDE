# ADR 0056: Persistir a sessão ativa no encerramento da janela

- Status: Accepted
- Date: 2026-09-27

## Context

As transições de workspace persistem o contexto anterior antes de trocar de
root ou navegar entre contextos, mas a sessão também pode mudar sem nenhuma
transição: o utilizador pode abrir, fechar, salvar ou reordenar tabs e depois
encerrar a IDE.

Sem uma captura no encerramento, o contexto mantinha o último estado salvo
antes da alteração de workspace. O próximo arranque restaurava uma sessão
antiga, apesar de a janela ter mostrado uma sessão mais recente.

## Decision

`MainWindow::closeEvent` captura o estado da sessão ativa antes de persistir
geometria e estado visual da janela. A captura usa o contrato de
`EditorSessionController`: apenas ficheiros persistíveis entram no contexto e
o índice ativo é relativo a esses ficheiros.

O encerramento não cria uma nova ativação de workspace, não altera recents e
não dispara uma restauração; apenas atualiza o estado do contexto atualmente
ativo.

## Alternatives considered

### Confiar apenas nas transições de workspace

Rejeitado porque alterações normais de documentos podem ocorrer sem qualquer
troca de contexto.

### Persistir a cada sinal de tab

Rejeitado nesta etapa porque espalharia I/O de settings pelo lifecycle de
edição e aumentaria o custo de cada interação. O encerramento é um ponto
determinístico e suficiente para garantir recuperação após reinício.

### Fazer `ContextManager` observar diretamente as tabs

Rejeitado porque misturaria armazenamento de contextos com widgets e
document-sync; `MainWindow` continua a fornecer a captura através da seam
existente.

## Consequences

O último estado materializado da sessão é preservado quando a janela fecha.
Fechos abruptos que não entreguem `closeEvent` continuam fora deste contrato;
uma futura recuperação transacional poderá ser adicionada sem alterar a
semântica de ativação de workspace.

## Verification

`testMainWindowPersistsSessionOnClose` abre um ficheiro numa `MainWindow`,
fecha a janela e verifica que o `ContextManager` recebeu o caminho e o índice
ativo. A suite CMake/CTest cobre a integração com a sessão existente.
