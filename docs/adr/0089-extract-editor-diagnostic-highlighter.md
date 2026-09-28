# ADR 0089: Extrair a projeção visual de diagnostics do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor` acumulava o estado dos diagnostics, a conversão de posições LSP
para offsets Qt, a escolha de cores por severidade e a construção de
`ExtraSelection`. Essa política ficava junto de rendering, input, Vim,
completion e sincronização do documento.

A decisão é introduzir `EditorDiagnosticHighlighter`. O módulo recebe o
documento, os diagnostics e uma função de cores, e devolve as seleções visuais.
Ele limita posições de linha e coluna ao documento disponível, protegendo o
widget contra ranges inválidos ou fora de data enviados pelo servidor.

## Considered Options

- **Manter a projeção em `CodeEditor`**: rejeitado porque mistura validação de
  protocolo com rendering e dificulta testar ranges inválidos sem configurar o
  widget inteiro.
- **Corrigir apenas os offsets dentro do widget**: rejeitado porque mantém a
  política de conversão escondida no monólito e não cria uma seam de teste.
- **Extrair `EditorDiagnosticHighlighter`**: escolhido porque concentra a
  transformação completa atrás de uma operação pura no que diz respeito ao
  estado do editor.

## Consequences

- `CodeEditor` continua dono do documento, dos diagnostics armazenados e da
  composição final de seleções, mas não conhece a conversão de ranges.
- A política de tolerância a ranges inválidos é local, explícita e testável.
- As cores continuam configuráveis pelo `CodeEditor`, evitando acoplamento do
  highlighter ao singleton de temas.

## Verification

- `cmake --build build -j 2`
- teste direto de ranges fora dos limites
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
