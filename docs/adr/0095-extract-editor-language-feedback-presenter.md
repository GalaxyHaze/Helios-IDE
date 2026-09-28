# ADR 0095: Separar apresentação de feedback LSP do controller de features

- Status: Accepted
- Date: 2026-09-27

`EditorLanguageFeatureController` misturava duas responsabilidades: iniciar
requests a partir de teclado, mouse, timers e menus, e apresentar resultados
LSP através de diagnostics, tooltips, navegação e document highlights. Isso
fazia a interface do controller conhecer simultaneamente precondições de
features e detalhes de apresentação Qt.

A decisão é criar `EditorLanguageFeedbackPresenter` como uma seam de
apresentação por editor. O presenter aceita um `LspClient`, conecta os
resultados relevantes, filtra URI/versão e delega a projeção visual para a
interface de `CodeEditor`. O controller permanece dono da interação e da
iniciação de requests.

## Considered Options

- **Manter requests e feedback no mesmo controller**: rejeitado porque
  qualquer alteração visual obrigaria a navegar por uma política de interação
  já complexa.
- **Fazer cada painel ou widget conectar-se diretamente ao `LspClient`**:
  rejeitado porque espalharia filtros de URI/versão e criaria múltiplos donos
  da apresentação de um documento.
- **Extrair um presenter por editor**: escolhido porque concentra a política
  de aceitação e projeção atrás de uma interface pequena (`attach`/`detach`),
  preservando a localidade do feedback.

## Consequences

- O controller de features concentra input, disponibilidade e requests.
- O presenter concentra conexões de resultados, filtros de stale responses e
  apresentação de feedback local.
- `CodeEditor` expõe uma operação explícita para aplicar ranges de highlights
  LSP, em vez de permitir que outro módulo mute seu estado privado.
- A navegação continua sendo um intent emitido pelo editor; o presenter não
  conhece a shell nem abre arquivos diretamente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
