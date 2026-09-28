# ADR 0060: Separar seleção de diretório da ativação de workspace

- Status: Accepted
- Date: 2026-09-27

## Context

O workspace podia ser escolhido a partir do Welcome, da árvore de ficheiros,
dos comandos `Open Folder` e `New Project`, e da navegação `Alt+Right`. A
seleção visual estava em `MainWindow`, enquanto a ativação já tinha sido
centralizada em `WorkspaceRootController`. `Alt+Right` ainda recebia callbacks
genéricos para pedir um caminho e criar um contexto, o que permitia que a
política de diálogo e a política de ativação voltassem a divergir.

Há duas decisões diferentes nesse fluxo:

1. qual diretório o utilizador quer selecionar, incluindo título e diretório
   inicial do diálogo;
2. se o diretório válido substitui o root atual ou cria um novo contexto,
   incluindo salvar o contexto anterior e persistir recents.

Misturar as decisões no composition root torna a política difícil de testar e
permite mutações parciais quando a seleção é cancelada ou inválida.

## Decision

`WorkspaceRootInteractionController` traduz intents de seleção em operações
semânticas de `WorkspaceRootController`:

- `openFolder()` seleciona com o título `Open Folder`, inicia no root atual
  quando existe e substitui o root do contexto;
- `newProject()` seleciona com o título `New project folder` e cria um novo
  contexto;
- `createContext()` reutiliza a mesma política de criação para `Alt+Right`.

O controller recebe uma função `RequestDirectory` opcional. A implementação
de produção usa `QFileDialog`; os testes fornecem um adapter determinístico.
Uma seleção vazia retorna `Rejected` antes de chamar o ativador. A validação,
normalização, salvamento do contexto e persistência de recents permanecem
exclusivamente em `WorkspaceRootController`.

`ContextNavigationController` recebe o módulo de interação, não callbacks
separados para pedir um caminho e criar um contexto. `WorkspaceNavigationController`
continua a ser um módulo de entrada de eventos: encaminha intents para a
interação, mas não conhece a política de diretórios.

## Alternatives considered

### Manter `QFileDialog` em `MainWindow`

Rejeitado porque mantém a política de seleção no composition root e força a
navegação de contextos a possuir um segundo caminho para a mesma interação.

### Colocar os diálogos em `WorkspaceRootController`

Rejeitado porque mistura interação visual com a transição de estado. O
ativador deve continuar testável sem widgets e sem diálogos.

### Manter callbacks `RequestWorkspaceRoot` e `CreateContext`

Rejeitado porque expõe duas operações relacionadas como callbacks
independentes. Um caller poderia pedir o caminho e esquecer a ativação, ou
escolher um título e diretório inicial diferentes dos demais caminhos.

## Consequences

Todos os intents de escolha de workspace passam por uma seam de interação
comum, enquanto a mutação de contexto continua numa seam de domínio separada.
`MainWindow` deixa de conter `openFolder()` e `newProject()`, reduzindo a
quantidade de política no composition root. Os testes podem verificar títulos,
diretórios iniciais, cancelamento e a operação de ativação sem abrir uma
janela.

O controller depende de `QFileDialog` apenas como adapter de produção. Se no
futuro houver uma superfície não-Qt para escolher diretórios, ela pode fornecer
`RequestDirectory` sem alterar a política de ativação.

## Verification

`testWorkspaceRootInteractionControllerCentralizesDirectorySelection`
verifica títulos, diretórios iniciais, substituição do root e criação de
contexto.

`testContextNavigationControllerPreservesHistoryPolicy` verifica que
`Alt+Right` usa o mesmo seam de criação de contexto e que uma seleção cancelada
não altera o histórico.

`testWorkspaceRootControllerRejectsInvalidRootsWithoutEffects` mantém a
verificação de que validação falha antes de salvar ou persistir recents.
