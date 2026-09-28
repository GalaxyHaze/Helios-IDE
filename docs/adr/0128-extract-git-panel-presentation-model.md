# ADR 0128: Extrair o modelo de apresentação do painel Git

- Status: Accepted
- Date: 2026-09-28

`GitPanel` já delegava o workflow de Git para `GitRepositorySession` e a lista
de ficheiros para `GitStatusListPresenter`, mas ainda decidia diretamente,
dentro do widget, a combinação de branch, resumo, visibilidade dos botões,
visibilidade da lista e estado busy. Isso deixava a semântica da superfície
espalhada por callbacks Qt e permitia estados incoerentes, como mostrar
“Connect to GitHub” quando não havia repositório ativo.

A decisão é introduzir `GitPanelPresentationModel`, um módulo puro que projeta
`GitRepositoryState` em `GitPanelPresentationState`. O modelo concentra as
regras de apresentação de estado; `GitPanel` continua sendo o adapter Qt que
aplica a projeção aos widgets, recebe intenções do utilizador e encaminha
operações para a sessão.

## Alternativas

- Manter as condições no `GitPanel`: rejeitado porque a apresentação continuaria
  acoplada ao lifecycle de widgets e seria difícil testar estados combinados.
- Colocar branch, resumo e botões no `GitStatusListPresenter`: rejeitado porque
  esse presenter deve continuar profundo para a lista, não conhecer o layout
  inteiro do painel.
- Criar um controller Qt para cada grupo de widgets: rejeitado porque
  aumentaria o número de seams sem separar uma política coerente.

## Consequências

- A regra de disponibilidade de ações fica testável sem criar um widget.
- O painel aplica um snapshot de apresentação coerente em vez de reconstruir
  condições em vários handlers.
- O presenter da lista mantém ownership apenas da lista e da seleção.
- Mensagens transitórias vindas da sessão continuam podendo substituir o
  resumo através da interface de feedback do painel.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `GitPanelPresentationModel` para no-repository, repository
  changed e repository connected
- `git diff --check`
