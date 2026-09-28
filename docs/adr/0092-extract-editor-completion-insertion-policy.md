# ADR 0092: Extrair a política de inserção de completion do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor` ainda calculava diretamente o início da palavra a substituir e
expandia placeholders de snippets quando o utilizador aceitava um item de
completion. Essa lógica é independente da apresentação Qt, mas estava
misturada com a mutação do `QTextDocument`.

A decisão é criar `EditorCompletionInsertionPolicy` como módulo puro. A sua
interface recebe o texto da linha, a posição UTF-16 do cursor, o texto de
inserção e o formato LSP, devolvendo o intervalo a substituir e o texto
expandido. `CodeEditor` continua dono do cursor, do documento e da operação de
inserção.

## Considered Options

- **Manter a lógica no `CodeEditor`**: rejeitado porque torna a política de
  snippet e delimitação de palavras dependente de um widget e dificulta testar
  limites de posição sem montar a UI.
- **Criar um controller para completion**: rejeitado porque duplicaria a seam
  já existente de `EditorLanguageFeatureController` e misturaria requests LSP
  com uma transformação local.
- **Extrair uma política pura**: escolhida porque concentra a decisão
  determinística atrás de uma interface pequena e deixa a mutação no dono do
  documento.

## Consequences

- A delimitação da palavra e a expansão de tabstops têm testes diretos.
- `CodeEditor` mantém a responsabilidade de aplicar a decisão ao documento.
- A política usa índices UTF-16 compatíveis com `QString` e `QTextCursor`.
- O formato de snippet continua limitado à expansão já suportada pelo editor;
  navegação entre tabstops permanece fora desta extração.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
