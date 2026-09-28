# ADR 0090: Extrair a aplicação de text edits no editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor::applyEdits` misturava a conversão de posições LSP, a ordenação
descendente de ranges e a mutação de `QTextDocument`. A mesma operação é
acionada por workspace edits, code actions e resultados de language services,
mas a política ficava escondida no widget.

A decisão é introduzir `EditorTextEditApplier`. O módulo recebe um documento e
uma lista de pares `LspRange`/texto, limita posições ao documento, ordena as
alterações do fim para o início e aplica tudo num único edit block. O
`CodeEditor` preserva os métodos públicos existentes como uma fachada fina.

## Considered Options

- **Manter a lógica no `CodeEditor`**: rejeitado porque cada consumidor de
  workspace edits dependeria de uma política incidentalmente localizada no
  widget.
- **Extrair apenas `offsetForLspPosition`**: rejeitado porque deixaria
  ordenação e atomicidade espalhadas ou ainda acopladas ao widget.
- **Extrair `EditorTextEditApplier`**: escolhido porque encapsula a
  transformação completa e torna ranges inválidos testáveis sem uma janela.

## Consequences

- A conversão e a ordem dos edits têm uma única implementação.
- `CodeEditor` mantém ownership do documento e a interface usada pelos
  controllers; o módulo não conhece widgets, LSP clients ou UI.
- A aplicação em ordem descendente continua preservando offsets de edits ainda
  não aplicados.

## Verification

- `cmake --build build -j 2`
- teste direto de offsets, clamping e edição descendente
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
