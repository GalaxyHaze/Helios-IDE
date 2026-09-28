# ADR 0065: Nomear as opções de arranque do LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`LspClient::start` recebia quatro `QString` consecutivas: caminho do servidor,
stdlib, root do workspace e modo de inicialização. O contrato dependia da
ordem, e `workspaceRoot` e `initMode` eram fáceis de trocar sem erro de
compilação. Lifecycle coordinators diferentes repetiam essa composição com
defaults implícitos.

## Decision

Introduzir `LspStartOptions` como value object nomeado com:

- `serverPath`;
- `stdlibPath`;
- `workspaceRoot`;
- `initMode`.

`LspClient::start(const LspStartOptions &)` é a interface usada por código
interno. O overload antigo permanece como adapter de compatibilidade e apenas
constrói o value object, evitando duas políticas de arranque.

Os coordinators de clangd e Zith constroem explicitamente as opções na seam de
composição. `LspClient` continua dono da validação de path e do lifecycle do
processo.

## Alternatives considered

### Manter argumentos posicionais

Rejeitado porque permite erros silenciosos entre valores do mesmo tipo e
espalha defaults pelos callers.

### Criar uma hierarquia de configurações para cada backend

Rejeitado porque Zith e clangd compartilham o mesmo contrato de processo e
apenas diferem no modo de inicialização. Uma hierarquia seria mais ampla que a
variação existente.

### Remover imediatamente o overload antigo

Rejeitado para esta tranche porque tests e integrações externas podem depender
da forma anterior. O overload é um adapter fino; novos callers devem usar as
opções nomeadas.

## Consequences

Callers internos ficam mais legíveis e a adição de uma nova opção não exige
mais uma sequência de parâmetros posicionais em todos os pontos. A interface
continua Qt-friendly e sem introduzir dependência em widgets. O overload de
compatibilidade deve ser removido apenas quando a superfície externa for
versionada ou quando não houver consumidores fora do repositório.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- clangd e Zith runtime compilam através de `LspStartOptions`
- `git diff --check`
