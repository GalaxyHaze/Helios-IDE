# ADR 0137: Dependências nomeadas para a interação do editor

- Status: Accepted
- Date: 2026-09-28

`EditorInteractionController` recebia três colaboradores posicionais para
ações LSP, chrome do editor e navegação de localizações, além de callbacks
nomeados para a política das ações Vim. Esses colaboradores são superfícies
estáveis que o controller coordena quando um editor é anexado; a ordem
posicional não acrescentava informação útil ao seam.

A decisão é agrupá-los em `EditorInteractionController::Dependencies`. Os
callbacks permanecem em `Callbacks` porque são políticas do composition root:
selecionar o editor atual, salvar, fechar, atualizar o estado e publicar
feedback. O controller continua sem ownership dos colaboradores.

## Alternativas

- Manter os três ponteiros posicionais: rejeitado porque o wiring escondia o
  papel de cada colaborador.
- Agrupar colaboradores e callbacks num contexto de janela: rejeitado porque
  ampliaria o seam e esconderia ownership.
- Fazer o controller possuir as ações LSP ou o navegador: rejeitado porque
  esses objetos têm lifecycle coordenado por `MainWindow`.

## Consequências

- A composição do editor fica auditável por campo.
- Os testes podem declarar explicitamente que a interação funciona sem esses
  adapters opcionais.
- O comportamento de attach e dos comandos Vim permanece inalterado.
- A interface ganha uma estrutura pequena, sem extrair uma classe artificial.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
