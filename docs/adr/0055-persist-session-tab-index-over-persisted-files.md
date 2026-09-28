# ADR 0055: Persistir o índice da tab ativa entre ficheiros persistíveis

- Status: Accepted
- Date: 2026-09-27

## Context

`EditorSessionState` guarda os caminhos dos ficheiros que podem ser
restaurados, mas a sessão também permite tabs untitled que não têm caminho
persistível. Usar diretamente o índice do `QTabWidget` fazia os dois índices
parecerem equivalentes.

Quando uma tab untitled aparecia antes de um ficheiro, o índice capturado
incluía essa tab, enquanto a restauração reconstruía apenas os ficheiros.
Consequentemente, a tab ativa podia mudar para outro documento depois de uma
troca de contexto ou de um novo arranque.

## Decision

`EditorSessionState::currentTab` é o índice da tab ativa dentro de
`EditorSessionState::openFiles`, e não o índice bruto do widget. Tabs untitled
não entram em `openFiles` e não alteram a contagem usada para calcular
`currentTab`.

Se a tab ativa for untitled, nenhum índice persistível é marcado; a
restauração mantém a seleção padrão produzida pela abertura dos ficheiros
persistidos. A sessão continua a restaurar apenas caminhos, não o estado
completo dos widgets.

## Alternatives considered

### Persistir o índice bruto do `QTabWidget`

Rejeitado porque o conjunto de tabs reconstruído não contém necessariamente
as tabs untitled presentes no momento da captura.

### Persistir o caminho do ficheiro ativo

Seria semanticamente explícito, mas exigiria alterar o value object persistido
e a compatibilidade do estado já produzido. O índice relativo ao conjunto
persistível mantém o formato atual e é suficiente para a restauração.

### Persistir tabs untitled

Rejeitado porque uma tab sem caminho não pode ser restaurada sem persistir
conteúdo, estado modificado e uma política adicional de recuperação.

## Consequences

A captura e a restauração compartilham o mesmo espaço de índices. A ordem das
tabs persistíveis é preservada, enquanto tabs untitled permanecem uma
preocupação transitória da sessão ativa. Qualquer futura alteração ao formato
de `EditorSessionState` deve preservar explicitamente essa distinção.

## Verification

`testEditorSessionControllerCapturesAndRestoresDocuments` captura uma sessão
com uma tab untitled antes de dois ficheiros, restaura-a e verifica que o
segundo ficheiro continua ativo. A suite CMake/CTest valida o contrato junto
com as restantes transições de sessão.
