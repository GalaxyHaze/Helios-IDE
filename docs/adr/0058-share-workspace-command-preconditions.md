# ADR 0058: Partilhar os pré-requisitos de comandos de workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`WorkspaceCommandController` e
`WorkspaceCommandAvailabilityController` precisavam de responder à mesma
pergunta: se Build, Check, Run ou Stop pode ser executado no estado atual.
Embora ambos consultassem a capacidade do LSP através da mesma seam, a
combinação de editor ativo, root de workspace, caminho persistido e tarefa em
execução era recalculada em dois módulos.

Essa duplicação permitia que uma ação aparecesse habilitada e fosse recusada
pelo executor, ou que uma ação fosse desabilitada apesar de o executor a
aceitar.

## Decision

`WorkspaceCommandController::canExecute(ShellCommand)` é a decisão semântica
partilhada para os comandos de workspace que o controller executa. Ele mantém
os pré-requisitos de Build, Check, Run e Stop junto da política de execução.

`WorkspaceCommandAvailabilityController` continua responsável por construir o
value object de disponibilidade e pelos tooltips, mas consulta essa decisão
através de um adapter. O fallback local permanece apenas para hosts de teste
ou composição que ainda não forneça o adapter; a composição da IDE fornece
sempre o executor real.

Format continua fora dessa decisão porque é uma ação LSP de documento e não é
executada pelo `WorkspaceCommandController`.

## Alternatives considered

### Manter as duas decisões booleanas

Rejeitado porque qualquer alteração de pré-requisito teria de ser sincronizada
manualmente entre execução e apresentação.

### Colocar tooltips no executor

Rejeitado porque misturaria política de execução com texto e apresentação do
shell.

### Criar um avaliador genérico para todas as ações

Rejeitado porque incluiria Format e ações não relacionadas ao workspace,
produzindo uma interface larga e menos profunda.

## Consequences

Uma mudança nos pré-requisitos de um comando de workspace passa a ser feita no
executor e é refletida pela disponibilidade. A disponibilidade ainda pode
explicar a indisponibilidade com mensagens próprias, mas não decide novamente
o resultado booleano quando está ligada à composição real.

## Verification

`testWorkspaceCommandAvailabilityTracksEditorAndTaskState` cobre a projeção
dos estados através do adapter semântico. O build CMake e a suite CTest devem
continuar a validar tanto a execução como a disponibilidade.
