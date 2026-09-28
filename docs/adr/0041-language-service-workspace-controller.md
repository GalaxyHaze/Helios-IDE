# ADR 0041: Isolar routing e reconciliação dos language services do workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` ainda aplicava enablement aos bindings de
`LspDocumentCoordinator`, resolvia clientes por path, identificava editores
C-family e iterava tabs para construir `ClangdLifecycleCoordinator::Configuration`.
Essas operações eram chamadas por session, disponibilidade de comandos,
transições de contexto e mudanças de settings.

Os módulos de protocolo, sincronização e lifecycle já existiam, mas a política
que os conectava ao estado do workspace continuava no composition root.

## Decision

`LanguageServiceWorkspaceController` passa a possuir a política de workspace:

- atualiza bindings Zith/C-family conforme enablement e caminho do clangd;
- resolve clientes por linguagem e path;
- reconhece a linguagem de editores;
- conta documentos C-family abertos;
- constrói e reconcilia a configuração do clangd;
- notifica a apresentação depois da reconciliação.

`MainWindow` fornece callbacks para settings, descoberta do executável, raiz do
workspace e refresh visual. `LspDocumentCoordinator` continua dono da
sincronização por documento e `ClangdLifecycleCoordinator` continua dono do
processo; o novo controller coordena a configuração entre eles.

## Alternatives considered

### Manter a configuração em `MainWindow`

Seria funcional, mas repetiria a combinação de settings, bindings e tabs em
cada chamada de abertura, mudança de contexto ou alteração de configuração.

### Colocar settings no `LspDocumentCoordinator`

Misturaria armazenamento de preferências e processo de descoberta de clangd
com a sincronização de documentos.

### Fazer `ClangdLifecycleCoordinator` contar tabs

Faria o lifecycle conhecer widgets e documentos visíveis, quebrando sua
interface de configuração explícita e a separação de responsabilidades.

## Consequences

A política de routing do workspace tem uma única interpretação e um teste
independente da janela. A interface de `MainWindow` fica menor e novos
language services podem reutilizar a seam de configuração sem adicionar mais
branches ao composition root. O controller continua Qt-facing porque precisa
observar tabs materializadas para determinar a configuração do clangd.

## Verification

`testLanguageServiceWorkspaceControllerCoordinatesRouting` verifica bindings
por path, reconhecimento C-family e as transições Disabled/MissingPath do
clangd.
