# ADR 0081: Extrair a política de deleção do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor::keyPressEvent` misturava a decisão de Backspace com a mutação de
`QTextCursor`: apagar o par auto-fechado quando o cursor está entre os
caracteres e remover um tab stop de quatro espaços quando a linha contém
apenas indentação. A decisão é introduzir `EditorDeletionPolicy`, que recebe
apenas o texto anterior ao cursor, o próximo carácter e a existência de
seleção, devolvendo uma `DeletionDecision`; `CodeEditor` continua a aplicar a
mutação e o comportamento Qt padrão.

## Consequences

- As regras de deleção são testáveis sem widget, documento Qt ou LSP.
- A política de pares de deleção fica separada da política de inserção em
  `EditorTypingPolicy`; cada módulo tem uma interface menor e uma razão de
  mudança única.
- A ordem do dispatch de teclas não muda: language features e Vim continuam
  precedendo Backspace.
- O editor mantém controle sobre cursores, seleção e efeitos no documento.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `EditorDeletionPolicy`
- `git diff --check`
