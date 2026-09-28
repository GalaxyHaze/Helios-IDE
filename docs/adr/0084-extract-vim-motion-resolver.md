# ADR 0084: Extrair a resolução de movimentos Vim

- Status: Accepted
- Date: 2026-09-27

`VimMotionController` mantinha dois mapas de movimentos repetíveis: um para
operações pendentes/visuais e outro no dispatch do modo normal. Esses mapas
tinham de permanecer sincronizados, embora a regra de resolução fosse a
mesma. Uma alteração em `w`, `b` ou num movimento direcional podia corrigir
um fluxo e deixar outro com semântica diferente.

A decisão é introduzir `VimMotionResolver`, um módulo puro que transforma uma
tecla num `VimMotion` nomeado. O módulo cobre apenas movimentos repetíveis
partilhados (`h`, `j`, `k`, `l`, `w`, `W`, `b`, `B` e `e`). O controller
continua responsável por aplicar o movimento ao documento e por tratar
comandos especiais que não são apenas movimentos repetíveis, como `0`, `^`,
`$` e `g`.

## Consequences

- A regra partilhada tem uma única seam e um teste direto sem widget Qt.
- Os fluxos normal, visual e de operação pendente usam o mesmo vocabulário.
- A aplicação da operação, a contagem efetiva e os efeitos de seleção
  continuam localizados no controller.
- `0`, `^`, `$` e `g` permanecem no controller porque têm semântica de comando
  adicional e não são equivalentes a um movimento repetível simples.
- O módulo não tenta modelar todo o Vim; a interface deliberadamente cobre
  apenas o conjunto que já tem mais de um consumidor.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `VimMotionResolver`
- `git diff --check`
