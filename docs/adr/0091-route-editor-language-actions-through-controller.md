# ADR 0091: Encaminhar ações de linguagem pelo controller do editor

- Status: Accepted
- Date: 2026-09-27

`CodeEditor` já delegava completion, hover e navegação para
`EditorLanguageFeatureController`, mas o menu de contexto continuava a chamar
`LspClient` diretamente para references e formatting e a emitir intents de
rename/code actions com regras próprias. Isso fazia o widget conhecer duas
formas de iniciar language features e duplicava a política de flush, posição e
validação de cliente.

A decisão é ampliar `EditorLanguageFeatureController` para possuir as
operações semânticas ligadas a um editor: references, formatting, rename e
code actions. O menu continua a construir e apresentar `QAction`, mas delega
as operações ao controller. `EditorLspActionController` continua responsável
por routing de intents de rename/code actions para os routers de aplicação.

## Considered Options

- **Manter chamadas diretas no menu**: rejeitado porque mistura apresentação
  Qt com protocolo LSP e cria uma segunda seam para a mesma feature.
- **Mover todo o menu para um novo controller**: rejeitado porque deslocaria
  composição visual sem melhorar a interface de language features.
- **Expandir `EditorLanguageFeatureController`**: escolhido porque é a seam
  existente para operações dependentes do editor, cursor, versão e cliente.

## Consequences

- Há uma única política editor-local para flush, URI, versão e posição das
  requests de language features.
- `CodeEditor` deixa de conhecer detalhes de construção de requests para
  references e formatting.
- A apresentação de ações e as decisões de routing continuam separadas:
  controller do editor inicia a intenção; `EditorLspActionController` e os
  routers aplicam a consequência no workspace.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
