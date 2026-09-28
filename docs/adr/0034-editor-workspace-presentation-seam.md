# ADR 0034: Extrair a política de apresentação do workspace do editor

- Status: Accepted
- Date: 2026-09-27

## Context

O shell central tinha duas responsabilidades misturadas em `MainWindow`:
decidir quando mostrar a welcome surface ou a área de edição e preparar a
find/replace bar para o editor ativo. A política era repetida por mudanças de
tab, fecho de editores, comandos de Find e callbacks de outros módulos.

Essa dispersão fazia com que uma mudança de sessão pudesse esquecer a
transição visual correspondente ou deixar a find bar associada a um editor
antigo. A welcome widget, a área de edição e a find bar são superfícies
visuais; nenhuma delas deve ser dona da política que as mantém coerentes.

## Decision

`EditorWorkspacePresentationController` passa a possuir a política de
apresentação do workspace central. A sua interface é composta por
`synchronize`, `showFind`, `showReplace`, `findNext` e `findPrevious`.

O módulo decide a superfície central a partir do número de tabs, esconde
breadcrumbs e find bar quando não existe editor materializado, e associa cada
comando de pesquisa ao editor ativo antes de o encaminhar. `MainWindow`
continua a compor os widgets, a criar e fechar editores e a persistir a sessão;
não replica a política visual.

## Alternatives considered

### Manter a política em `MainWindow`

Evitaria um novo seam, mas manteria duplicação entre sinais de tabs,
callbacks de sessão e comandos do shell. Também aumentaria o risco de estados
visuais inconsistentes.

### Criar um controlador por widget

Separaria chamadas de apresentação, mas produziria módulos rasos sem uma
política própria. O seam escolhido agrupa a transição central e a coerência da
find bar porque ambas dependem da mesma condição: a existência e identidade
do editor ativo.

### Fazer a sessão de editores possuir a apresentação

Misturaria materialização de documentos com a política visual do shell. A
sessão pode ser reutilizada em testes ou outra superfície sem precisar de
widgets de welcome, breadcrumbs ou find.

## Consequences

A transição welcome/editor e os comandos de pesquisa têm uma única fonte de
verdade, e os testes atravessam uma interface pequena. O controlador conhece
as superfícies Qt do shell, mas não conhece `ContextManager`, LSP, settings ou
documentos. Novas superfícies centrais devem ser introduzidas através desta
política, em vez de adicionar mais condicionais a `MainWindow`.

## Verification

O teste `testEditorWorkspacePresentationKeepsCentralModeAndFindPolicy` verifica
a transição sem tabs, a associação da find bar ao editor ativo e o retorno à
welcome surface depois de remover a última tab.
