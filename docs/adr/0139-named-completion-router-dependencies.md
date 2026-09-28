# ADR 0139: Dependências nomeadas para o router de completion

- Status: Accepted
- Date: 2026-09-28

`LspCompletionRouter` recebia quatro colaboradores de apresentação e modelo
em posições fixas: tabs, snippets, completer e modelo de completion. O router
tem uma política coesa de aceitar apenas resultados do editor ativo, combinar
snippets e abrir a projeção de completion; porém, o wiring não comunicava o
papel de cada colaborador.

A decisão é agrupar esses colaboradores em
`LspCompletionRouter::Dependencies`. A política `isEnabled` permanece
separada, pois é uma consulta do host e não um objeto colaborador. O router
continua ligado aos resultados específicos de completion do `LspClient`; este
ADR não amplia o seam de eventos LSP da shell.

## Alternativas

- Manter argumentos posicionais: rejeitado porque a alteração da superfície de
  completion poderia ligar um modelo ou widget errado sem erro de tipo.
- Agrupar também `isEnabled`: rejeitado porque misturaria dependência
  persistente com política dinâmica do composition root.
- Criar um router genérico para todos os resultados LSP: rejeitado porque
  completion tem invariantes próprias de editor ativo, snippets e completer.

## Consequências

- O wiring do router fica explícito e auditável.
- Os testes montam a superfície de completion por campos nomeados.
- A interface de eventos da shell não é ampliada especulativamente.
- Routers menores continuam inalterados até existir risco concreto equivalente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
