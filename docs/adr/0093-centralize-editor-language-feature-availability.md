# ADR 0093: Centralizar disponibilidade de language features no editor

- Status: Accepted
- Date: 2026-09-27

O menu contextual e os atalhos de `CodeEditor` consultavam diretamente
`LspClient::isReady()` e vários métodos `has*Provider()`. As requests já eram
encaminhadas por `EditorLanguageFeatureController`, mas a decisão de
disponibilidade estava duplicada no widget e os atalhos podiam consumir uma
tecla mesmo quando o provider não existia.

A decisão é expor no controller uma interface pequena baseada em
`EditorLanguageFeatureController::Feature`. `isAvailable` combina readiness e
capability negociada; menu, Ctrl-click, atalhos e timers editor-locais
consultam essa mesma seam.
O controller continua iniciando as requests e o widget continua construindo
as ações visuais.

## Considered Options

- **Manter os checks no `CodeEditor`**: rejeitado porque deixa a apresentação
  conhecer detalhes do protocolo e permite precondições divergentes.
- **Expor todos os `has*Provider()` no `CodeEditor`**: rejeitado porque
  aumentaria a interface pública e espalharia a mesma política.
- **Centralizar uma enumeração de features no controller**: escolhida porque
  mantém uma interface estreita e um único ponto para readiness mais
  capability.

## Consequences

- Menu, atalhos, Ctrl-click e projeções assíncronas concordam sobre quando uma
  feature está disponível.
- O widget deixa de depender diretamente dos nomes das capabilities LSP para
  apresentar ações.
- Adicionar uma feature exige atualizar um único enum e o mapeamento do
  controller. Provider capabilities que ainda não eram modeladas devem ser
  adicionadas ao value object LSP antes de serem consumidas pela seam.
- A capability de formatting agora controla também a habilitação do menu,
  evitando oferecer uma ação que o servidor não anuncia.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
