# ADR 0043: Isolar a coordenação dos eventos do LSP runtime

- Status: Accepted
- Date: 2026-09-27

## Context

O construtor de `MainWindow` interpretava diretamente eventos de dois clientes
LSP e do runtime Zith. As lambdas combinavam conexão, paragem, erros, logs,
anexação de documentos, estado do `clangd`, status bar e disponibilidade de
ações.

Além de aumentar a responsabilidade do composition root, parte dessa ligação
era feita antes de `WorkspaceCommandController` ser construído. Isso tornava
fácil criar uma conexão com uma dependência ainda nula e deixava a ordem de
construção implícita.

## Decision

`LspRuntimeEventController` passa a possuir a coordenação de eventos:

- subscreve sinais do runtime Zith e dos clientes Zith/clangd;
- projeta conexão, paragem e erro na apresentação e no status;
- mantém o erro do runtime através de `LspRuntimeController`;
- anexa e desanexa documentos quando os servidores ficam prontos ou param;
- atualiza `ClangdLifecycleCoordinator`;
- encaminha logs e disponibilidade de ações;
- conecta-se somente depois de `WorkspaceCommandController` e dos demais
  módulos necessários estarem construídos.

`LspRuntimeController` continua dono das transições de enablement, refresh e
limpeza de cache. `LspRuntimePresentationController` continua dono da
projeção visual. Os clientes e coordenadores continuam donos dos seus
processos e estados internos.

## Alternatives considered

### Manter lambdas no construtor

Seria menor em ficheiros, mas manteria a política espalhada e preservaria o
risco de conexões serem criadas antes das dependências correspondentes.

### Colocar os handlers em `LspRuntimeController`

Misturaria transições iniciadas pelo usuário com eventos assíncronos dos
processos, tornando a interface de enablement mais larga e menos previsível.

### Fazer cada cliente atualizar a UI diretamente

Acoplaria protocolo e lifecycle às superfícies da IDE, duplicando a política
de apresentação entre Zith e clangd.

## Consequences

A interpretação dos eventos tem uma única seam e pode ser testada emitindo
sinais controlados. A ordem de composição fica explícita: dependências são
construídas primeiro, depois o controller faz `attach()`. Novos eventos de
runtime devem ser adicionados ao controller, não a lambdas isoladas na janela.

## Verification

`testLspRuntimeEventControllerProjectsLifecycleEvents` verifica conexão,
erro, log e paragem através da interface do controller. A suite completa passa
com 76 testes.
