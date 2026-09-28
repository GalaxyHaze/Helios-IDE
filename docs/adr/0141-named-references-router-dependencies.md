# ADR 0141: Dependências nomeadas para o router de referências

- Status: Accepted
- Date: 2026-09-28

`LspReferencesRouter` recebia tabs e painel de referências como ponteiros
posicionais. Ambos são colaboradores persistentes com papéis distintos na
política de aceitar apenas resultados da tab ativa e da versão atual; a ordem
dos argumentos não comunicava esses papéis.

A decisão é introduzir `LspReferencesRouter::Dependencies` com campos
nomeados para `tabWidget` e `referencesPanel`. A função que abre o painel
continua separada porque é uma política do shell, não um colaborador
persistente do router. A invariável de URI/versão e a apresentação só quando
há referências permanecem dentro do módulo.

## Alternativas

- Manter ponteiros posicionais: rejeitado porque permitia wiring acidental de
  superfícies de apresentação e tornava a composição menos auditável.
- Agrupar também `showReferences`: rejeitado porque confundiria infraestrutura
  persistente com uma política de apresentação do host.
- Criar uma interface genérica para tabs/painéis: rejeitado porque não há
  segundo adapter nem vocabulário comum que justifique essa abstração.

## Consequências

- O composition root declara explicitamente os colaboradores do router.
- Os testes atravessam o mesmo seam com campos nomeados.
- O router continua dono da coerência assíncrona de URI e versão.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
