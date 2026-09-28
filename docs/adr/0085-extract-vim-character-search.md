# ADR 0085: Extrair a busca Vim por caractere

- Status: Accepted
- Date: 2026-09-27

`VimMotionController` misturava a interpretação dos comandos com a busca por
um caractere na linha. Essa busca tem regras próprias: começa depois ou antes
do cursor, repete ocorrências, suporta `t`/`T` e guarda o último caractere
para `;` e `,`. Ela é diferente da busca textual multilinear representada
por `VimSearchSession`.

A decisão é introduzir `VimCharacterSearch`, que recebe um
`VimCharacterSearchRequest`, aplica a busca no editor e mantém apenas o
estado necessário para repetir o último movimento. O controller continua
responsável por interpretar `f`, `F`, `t`, `T`, `;` e `,`, incluindo contagem
pendente e ordem de dispatch.

## Consequences

- O algoritmo de busca por caractere e a memória do último movimento têm uma
  única seam e podem ser testados sem `VimMotionController`.
- A busca textual e a busca por caractere permanecem conceitos distintos,
  evitando um módulo de busca genérico com interface ambígua.
- O controller não precisa conhecer o algoritmo de índices na linha.
- A semântica existente de ausência de ocorrência é preservada: a busca
  consome o comando, mas não move o cursor nem atualiza a memória de repetição.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- testes diretos de `VimCharacterSearch`
- `git diff --check`
