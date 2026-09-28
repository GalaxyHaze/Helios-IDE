# ADR 0040: Isolar a transição de contexto do workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` recebia `ContextManager::contextChanged` e executava uma política
com várias consequências: atualizava runtime, árvore de ficheiros, pesquisa,
Git e indicador de contexto; depois distinguia `RootChanged` das outras causas
para decidir restauração da sessão e revalidação de LSP/clangd.

Embora `ContextChangeReason` já tornasse a causa explícita, a política ainda
estava numa lambda extensa do composition root.

## Decision

`ContextWorkspaceController` subscreve `ContextManager::contextChanged` e
centraliza a transição. A sua interface recebe adapters para:

- aplicar uma nova raiz às superfícies de workspace;
- atualizar o indicador de contexto;
- restaurar a sessão persistida;
- consultar enablement e execução do LSP;
- revalidar runtime Zith e lifecycle clangd.

O controller sempre publica a raiz e o indicador. Para `RootChanged`, preserva
os editores abertos, mas ainda revalida os language services ativos, porque a
raiz do workspace faz parte da identidade dos servidores Zith e C-family. Para
`Navigation` e `NewContext`, restaura a sessão e, quando o LSP Zith está ativo,
solicita runtime preferindo cache, seguido da atualização do lifecycle clangd.
Em todos os motivos, a revalidação acontece depois de aplicar a nova raiz e
antes de concluir a transição.

`ContextManager` continua dono do armazenamento e da emissão do motivo; o
controller não passa a possuir sessões, painéis ou processos.

## Alternatives considered

### Manter a lambda em `MainWindow`

Preservaria a conexão existente, mas deixaria a regra de restauração e
revalidação espalhada no composition root, apesar de já existir um motivo
semântico explícito.

### Colocar a política em `ContextManager`

Faria o armazenamento de contextos conhecer widgets, sessões e lifecycle de
language services, quebrando a separação entre estado e shell.

### Criar um handler por superfície

Atualizaria cada painel isoladamente e perderia a invariável de que uma
transição deve atualizar todas as superfícies antes de decidir restauração.

## Consequences

A política de transição tem uma seam pequena e uma única interpretação de
`ContextChangeReason`. `MainWindow` fornece adapters concretos, mas deixa de
coordenar manualmente cada efeito da mudança. Novas superfícies de workspace
devem ser adicionadas ao adapter de raiz, sem duplicar a decisão de restauração.

## Verification

`testContextWorkspaceControllerPreservesTransitionSemantics` verifica troca
de raiz, criação de contexto e navegação, incluindo restauração e revalidação
condicionais.

`testClangdLifecycleCoordinatorRestartsWhenWorkspaceRootChanges` verifica que
uma mudança de raiz não reutiliza um processo clangd iniciado para outro
workspace.
