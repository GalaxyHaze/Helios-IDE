# ADR 0051: Distribuir o dispatch de comandos por domínio

- Status: Accepted
- Date: 2026-09-27

## Context

Embora `ShellCommandSurface` já emitisse um vocabulário semântico, o
`MainWindow::handleShellCommand` ainda interpretava todos os comandos:
execução de tarefas, pesquisa, visibilidade de painéis, navegação da sidebar,
diálogos, ficheiros e lifecycle.

Isso fazia a janela funcionar como um handler universal. Cada nova regra de um
domínio aumentava o `switch` do composition root e exigia que a janela
conhecesse detalhes de módulos que já possuíam a política correspondente.

## Decision

Os módulos que já possuem uma política de domínio podem expor um handler
limitado `handleShellCommand(ShellCommand)` que retorna se reconheceu a
intenção:

- `WorkspaceCommandController`: Build, Check File, Run e Stop;
- `EditorLspActionController`: Format Document;
- `LspRuntimeController`: Restart LSP;
- `WorkspacePanelPresentationController`: Outline e Bottom Panel;
- `EditorWorkspacePresentationController`: Find, Replace, Find Next e Find
  Previous;
- `SidebarController`: Explorer, Workspace Search, Git, Settings e Hide
  Sidebar.
- `ShellDialogController`: Preferences, Vim Help, Shortcuts, LSP Manager e
  Getting Started.
- `WorkspaceNavigationController`: New Project e Open Folder.

`MainWindow` delega primeiro a esses handlers e mantém apenas os comandos que
exigem composição da aplicação: criação/abertura/salvamento de documentos,
criação de uma nova janela e saída da aplicação.

O retorno booleano é deliberado: cada handler conhece apenas o seu subconjunto
e não há um registry genérico de callbacks. A janela mantém um `default`
defensivo para o caso de um domínio não estar composto.

## Alternatives considered

### Manter todo o `switch` em `MainWindow`

Rejeitado porque concentrava políticas independentes no composition root e
fazia o crescimento de cada domínio aumentar o monólito.

### Criar um `ShellCommandRouter` genérico

Rejeitado porque deslocaria o `switch` para outro módulo sem aumentar a
profundidade: o router teria de conhecer todos os domínios e possuir uma
interface larga de callbacks.

### Fazer `ShellCommandSurface` executar os comandos

Rejeitado porque misturaria apresentação de menus com sessões, runtime,
documentos e estado de workspace.

## Consequences

Cada domínio passa a ser testável através da sua própria interface de comando,
e `MainWindow` coordena apenas as decisões que atravessam fronteiras de
aplicação. Ao adicionar um novo comando, o proprietário da política deve
decidir se ele pertence a um handler existente ou a uma nova seam profunda,
antes de editar o composition root.

## Verification

Os testes existentes foram ampliados para exercitar o dispatch dos handlers de
workspace, painéis, editor e sidebar. O build CMake e o CTest completo passam
após a alteração; `clang-tidy` e `cppcheck` mantêm apenas avisos conhecidos de
headers Qt/MOC e modernização preexistente.
