# ADR 0131: Dependências nomeadas para o runtime LSP

- Status: Accepted
- Date: 2026-09-28

`LspRuntimeController` recebia oito objetos em posições fixas antes dos
callbacks. A lista combinava superfícies Qt, runtime Zith, cliente clangd e
projeções de estado, tornando a composição difícil de auditar e fácil de
corromper durante uma alteração.

A decisão é introduzir `LspRuntimeController::Dependencies`, com campos
nomeados para os colaboradores persistentes. O controller continua a possuir
as transições de enablement, refresh e limpeza; a mudança apenas torna a seam
de construção explícita. Os callbacks permanecem num agrupamento separado,
pois são políticas fornecidas pelo host e não ownership do controller.

## Alternativas

- Manter os argumentos posicionais: rejeitado pelo risco de wiring silencioso.
- Agrupar dependências e callbacks num único contexto: rejeitado porque
  misturaria objetos possuídos com políticas do composition root.
- Criar interfaces abstratas para todos os painéis: rejeitado porque não há
  adapters alternativos que justifiquem novas seams.

## Consequências

- A composição de runtime pode ser revista campo a campo.
- Os testes injetam apenas os colaboradores necessários para cada cenário.
- O custo é um pequeno bloco de composição explícita no `MainWindow`.
- Nenhuma regra de enablement ou ownership foi transferida.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
