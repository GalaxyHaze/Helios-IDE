# ADR 0078: Extrair o matching de brackets do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor::matchBrackets` misturava a decisão algorítmica de qual bracket
combinar com a criação de `QTextEdit::ExtraSelection`, formatos de cor e
acesso ao cursor Qt. Isso tornava uma alteração no algoritmo dependente do
widget e dificultava testar nesting, direção e ausência de par.

A decisão é introduzir `BracketMatcher`. A interface recebe uma linha e a
posição do cursor e devolve um `BracketMatch` com as duas posições ou um
resultado inválido. O módulo preserva a semântica atual: considera o bracket
fechado imediatamente antes do cursor, o bracket aberto imediatamente depois,
e conta apenas nesting do mesmo tipo.

`CodeEditor` continua responsável por limpar as seleções antigas, aplicar as
cores configuradas e converter as posições em seleções visuais.

## Consequences

- O algoritmo é puro e testável sem criar `CodeEditor` ou configurar uma
  paleta.
- A apresentação do matching fica localizada no editor, sem contaminar o
  módulo de cálculo.
- O resultado nomeado evita retornar posições paralelas sem significado.
- Outros tipos de bracket ou regras de linguagem podem evoluir dentro da seam
  sem alterar o rendering.
- Não foi introduzido parsing de sintaxe completo: a política continua
  deliberadamente limitada à semântica Vim/editor existente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `BracketMatcher`
- `git diff --check`
