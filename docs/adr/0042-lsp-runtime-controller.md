# ADR 0042: Isolar a política de enablement do LSP runtime

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` acumulava a transição entre LSP ativo e desativado, incluindo
persistência da preferência, enablement do runtime Zith, paragem do cliente
clangd, limpeza de completion/diagnostics/outline, atualização de status,
refresh do runtime, limpeza de cache e sincronização das ações do shell.

Essa política era chamada por dois painéis, pelo contexto do workspace e pelo
comando de restart. O routing por linguagem e o lifecycle dos processos já
tinham módulos próprios, mas a mudança de estado ainda estava espalhada no
composition root.

## Decision

`LspRuntimeController` passa a possuir a política de enablement e operação do
runtime:

- inicializa o estado ativo ou desativado;
- persiste a preferência e sincroniza os painéis;
- para o cliente clangd e limpa estados transitórios ao desativar;
- solicita refresh do runtime com preferência de cache;
- confirma e executa a limpeza do cache;
- mantém o último erro do runtime;
- atualiza reconciliação clangd, ações de shell e mensagens de status.

`MainWindow` fornece adapters para persistência, confirmação, reconciliação,
mensagens e atualização de ações. `LanguageServiceWorkspaceController`
continua dono do routing por linguagem e `ZithRuntimeLifecycleCoordinator`
continua dono do processo e do toolchain.

## Alternatives considered

### Manter a política em `MainWindow`

Seria funcional, mas cada origem de comando continuaria conhecendo a ordem
necessária para parar clientes, limpar views, atualizar apresentação e
reconciliar clangd.

### Colocar enablement em `ZithRuntimeLifecycleCoordinator`

Misturaria o lifecycle do processo Zith com a limpeza de views C-family, ações
do shell, persistência de preferências e confirmação de UI.

### Criar um controller genérico de todos os language services

Teria uma interface larga e rasa, misturando routing, documentos, processos e
apresentação. O seam foi limitado à política de enablement do runtime para
preservar profundidade e localidade.

## Consequences

A janela deixa de ser o local que interpreta a sequência de enablement e
disablement. Novos pontos de entrada podem usar a mesma política sem
duplicar efeitos colaterais. A confirmação e a persistência continuam
substituíveis através de adapters, permitindo testar a transição sem depender
de diálogos ou settings globais.

## Verification

`testLspRuntimeControllerOwnsDisableTransition` verifica a transição
desativada, persistência, reconciliação, atualização das ações e manutenção
do erro através da interface do controller.
