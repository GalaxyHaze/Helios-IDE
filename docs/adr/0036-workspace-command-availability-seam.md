# ADR 0036: Separar disponibilidade e execução de comandos de workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow::updateRunActionsEnabled` calculava diretamente se build, check,
format, run e stop eram possíveis. A mesma janela também encaminhava a
execução para `WorkspaceCommandController`, fazendo o composition root
conhecer simultaneamente capacidades do editor, estado do LSP, estado da
tarefa e detalhes de tooltip da superfície de menus.

Disponibilidade e execução mudam por razões diferentes: a execução acompanha
processos e resultados, enquanto a disponibilidade é uma projeção derivada do
editor ativo e do estado atual do runtime.

## Decision

`WorkspaceCommandController` passa a expor a interface semântica
`canExecuteWorkspaceCommand()`. Essa é a única decisão sobre a capacidade do
LSP Zith de executar comandos de workspace: verifica a política externa de
enablement, o estado pronto do cliente e o capability
`workspace/executeCommand`.

`WorkspaceCommandAvailabilityController` continua a derivar um
`WorkspaceCommandAvailability` e publicá-lo em `ShellCommandSurface`, mas
consulta a interface do executor em vez de repetir essa política.
Build e Run também exigem um workspace root ativo, pois a execução dessas
operações rejeita requests sem root.
Build, Check e Run também validam no executor que existe um editor Zith ativo;
isso impede que comandos de shell contornem o mesmo pré-requisito usado para
habilitar as ações da UI.
Stop depende apenas de um LSP executável e de uma tarefa ativa; não depende do
editor atualmente selecionado, porque uma tarefa pode continuar enquanto o
usuário muda de documento.

O módulo recebe callbacks mínimos para:

- saber se o editor é Zith;
- obter o editor ativo;
- obter o workspace root ativo;
- saber se existe uma tarefa em execução.

`WorkspaceCommandController` continua dono da execução, output, progress e
cleanup. O composition root fornece ao executor somente a política externa
`lspEnabled`; detalhes de prontidão e capabilities permanecem no executor.
`ShellCommandSurface` continua dono de menus, atalhos e ações; o value object
de disponibilidade permanece uma interface independente.

## Alternatives considered

### Manter cálculo e execução em `MainWindow`

Evitaria novos tipos, mas preservaria uma política extensa no composition root
e dificultaria testar combinações de editor, LSP e tarefa sem construir a IDE.

### Fazer `MainWindow` decidir a capacidade do LSP

Manteria a combinação de `lspEnabled`, prontidão e capabilities no
composition root e obrigaria a disponibilidade e a execução a dependerem de
callbacks diferentes. Isso permitiria divergência entre a ação habilitada e a
operação realmente aceita.

### Colocar disponibilidade de UI em `WorkspaceCommandController`

Misturaria uma projeção de UI com lifecycle de processos e faria o executor
conhecer tooltips e menus.

### Fazer `ShellCommandSurface` decidir disponibilidade

Daria à superfície visual conhecimento de editor, LSP e tarefas, invertendo a
dependência e tornando a mesma política impossível de reutilizar noutra
superfície.

## Consequences

As regras de capacidade do executor ficam localizadas e atravessam uma
interface testável. A disponibilidade da UI reutiliza essa decisão sem
conhecer o cliente LSP concreto. O shell ainda fornece callbacks de estado
específicos da projeção, mas não repete a combinação de prontidão e
capabilities. A lista de capabilities suportadas exige alteração explícita
quando um novo comando de workspace for adicionado.

## Verification

`testLspClientExecutesWorkspaceCommand` verifica que
`WorkspaceCommandController::canExecuteWorkspaceCommand()` aceita um cliente
Zith inicializado com `workspace/executeCommand` e rejeita a mesma situação
quando a política externa desabilita o LSP.

`testWorkspaceCommandAvailabilityTracksEditorAndTaskState` verifica a
habilitação inicial, a transição para tarefa em execução e a explicação de
indisponibilidade quando o executor não pode executar comandos.
