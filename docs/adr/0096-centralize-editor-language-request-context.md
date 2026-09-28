# ADR 0096: Centralizar o contexto de requests de linguagem no editor

- Status: Accepted
- Date: 2026-09-27

`EditorLanguageFeatureController` reconstruía repetidamente o mesmo contexto
para requests LSP: fazia flush das alterações, lia URI e versão privadas e
convertia o cursor para uma posição LSP. O controller também manipulava
diretamente o popup e o completer, exigindo um `friend` com `CodeEditor`.

A decisão é expor um `EditorLanguageRequestContext` como value object e fazer
`CodeEditor::currentLanguageRequest()` criar esse valor após o flush. A
interface também oferece operações de editor para preparar e tratar teclas do
popup de completion. O controller conserva a decisão de disponibilidade e o
encaminhamento para o `LspClient`, mas não conhece os campos privados do
documento ou do completer.

## Considered Options

- **Manter a reconstrução no controller**: rejeitado porque duplica
  invariantes de URI, versão e posição em vários requests.
- **Expor apenas `uri()`, `version()` e `cursorPosition()`**: rejeitado
  porque mantém a composição e o flush espalhados pelo caller.
- **Deixar o controller operar diretamente o completer via `friend`**:
  rejeitado porque acopla uma política de interação a detalhes do widget.
- **Devolver um contexto validável e operações semânticas do editor**:
  escolhido porque concentra invariantes no dono do documento e mantém a
  interface do controller orientada a requests.

## Consequences

- Requests LSP usam uma única fonte para URI, versão e posição.
- `CodeEditor` é responsável por sincronizar alterações pendentes antes de
  construir o contexto.
- O controller deixa de depender dos campos `m_fileUri` e `m_completer`;
  o `friend` de `EditorLanguageFeatureController` foi removido.
- O value object não contém o cliente LSP nem efeitos de rede, permitindo
  testar a validade do contexto sem iniciar um processo.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
