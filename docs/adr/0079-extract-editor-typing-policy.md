# ADR 0079: Extrair a política de digitação do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor::keyPressEvent` e os seus helpers misturavam a ordem de dispatch
de teclas com decisões de indentação e auto-close. A política de pares
incluía surround de seleção, salto sobre um fechamento existente e regras
específicas para aspas dentro de palavras; a indentação também tinha uma regra
própria para linhas terminadas em `{`.

A decisão é introduzir `EditorTypingPolicy`. O módulo recebe apenas valores
textuais e contexto mínimo do cursor, e devolve uma `AutoCloseDecision` ou a
string de indentação. `CodeEditor` continua responsável por executar a ação
no documento, reposicionar o cursor e disparar signature help quando
necessário.

## Consequences

- As regras de digitação podem ser testadas sem criar um widget ou servidor
  LSP.
- A ordem de prioridade de `CodeEditor::keyPressEvent` não muda: LSP e Vim
  continuam precedendo a política de digitação.
- A política não possui `QTextCursor`, não faz I/O e não conhece cores,
  snippets ou apresentação.
- A decisão nomeada evita espalhar booleanos para “tratou”, “saltou” e
  “inseriu”.
- A regra atual de quatro espaços após `{` fica explícita e localizada; uma
  futura configuração de indentação pode substituir o módulo sem reescrever
  o dispatch de teclas.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `EditorTypingPolicy`
- `git diff --check`
