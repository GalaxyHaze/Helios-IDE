# ADR 0044: Fazer o workspace-command controller possuir o event intake

- Status: Accepted
- Date: 2026-09-27
- Refinement: ADR-0120 moves process-output, process-exit, progress, and
  process-stop handling into `WorkspaceTaskOutputController`; command-result
  and save-all intake remain here.

## Context

`WorkspaceCommandController` já interpretava output de processos, exits,
progresso e resultados de comandos, mas `MainWindow` ainda ligava
manualmente os sinais do `LspClient` a esses handlers. O shell também ligava
`saveAllRequested` diretamente ao método da janela.

Isso fazia o composition root conhecer detalhes do protocolo de comandos e
permitia que a ligação entre cliente e controller ficasse duplicada ou
incompleta em novos pontos de composição.

## Decision

`WorkspaceCommandController` conecta o cliente recebido no seu construtor aos
handlers que já possui:

- `processOutputReceived`;
- `processExitReceived`;
- `workDoneProgressReceived`;
- `commandResult`;
- `processStopped`;
- `saveAllRequested`, delegado ao callback `saveAll`.

`MainWindow` continua fornecendo callbacks de persistência, apresentação e
estado do editor, mas deixa de conhecer a ligação dos eventos de protocolo
com a política de tarefas. Mudanças de estado do controller são expostas por
um único sinal `stateChanged`; a atualização de ações não atravessa um callback
paralelo.

`LspRuntimeEventController` não recebe nem conhece
`WorkspaceCommandController`; eventos de runtime e eventos de tarefas mantêm
seams distintas.

## Alternatives considered

### Manter as conexões em `MainWindow`

Exigiria repetir a lista de sinais em cada composition root e manteria o
conhecimento do protocolo fora do módulo que possui o estado de tarefas.

### Criar um router genérico de todos os eventos LSP

Misturaria lifecycle, runtime, diagnostics e comandos de workspace, criando
uma interface larga e reduzindo a localidade das regras.

### Fazer `LspClient` conhecer o controller

Acoplaria o cliente de protocolo a uma política específica da aplicação e
impediria reutilização do cliente com outros consumidores.

## Consequences

O controller passa a ser auto-contido para a entrada de eventos que já
interpreta. A janela fica menor e a ordem de composição é mais segura:
clientes e callbacks são fornecidos antes do wiring ser instalado. Testes
podem emitir sinais no cliente e atravessar a mesma interface usada em
produção.

## Verification

`testWorkspaceCommandControllerOwnsClientEventWiring` verifica `saveAll`,
output antecipado e resultado de comando através das conexões instaladas pelo
controller. A suite completa passa com 77 testes.
