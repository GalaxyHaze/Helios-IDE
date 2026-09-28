# ADR 0086: Extrair a sessão de operações pendentes Vim

- Status: Accepted
- Date: 2026-09-27

`VimMotionController` acumulava o estado e a execução de operações pendentes:
operador `d`, `c` ou `y`, contagem, âncora, motions, `g`/`G`, busca por
caractere e aplicação da seleção. Esse estado era resetado em vários caminhos
do controller e fazia a interpretação normal conhecer detalhes da seleção
pendente.

A decisão é introduzir `VimPendingOperationSession`. O módulo recebe o
editor e a capacidade de busca por caractere, inicia uma operação com
operador, contagem e âncora, e devolve um `VimPendingOperationResult` nomeado.
O resultado informa se o evento foi consumido e se a operação requer entrada
no modo Insert. O controller permanece dono da transição global de modo e da
ordem de dispatch.

## Considered Options

- **Manter o estado no controller**: rejeitado porque cada nova operação
  pendente aumenta os caminhos de reset e mistura seleção com dispatch normal.
- **Criar um controller por operador (`DeleteController`, `ChangeController`,
  etc.)**: rejeitado porque fragmentaria uma única máquina de estado e
  duplicaria a aplicação de motions.
- **Extrair uma sessão única de operações pendentes**: escolhido porque
  concentra invariantes de âncora, contagem, finalização e cancelamento atrás
  de uma interface curta.

## Consequences

- `VimMotionController` não conhece mais a representação da operação pendente,
  nem constrói seleções para `d/c/y`.
- A sessão pode ser testada diretamente com eventos e um editor mínimo.
- `VimDocumentOperations` continua sendo o adapter que muta o documento; a
  sessão decide quando e qual seleção aplicar.
- A mudança de modo permanece no controller através do resultado nomeado,
  evitando acoplar a sessão a sinais de apresentação.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `VimPendingOperationSession`
- `git diff --check`
