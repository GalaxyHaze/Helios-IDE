# ADR 0046: Isolar a política de fechamento de tabs

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` conectava `QTabWidget::tabCloseRequested` e, dentro da lambda,
decidia quando pedir confirmação, quando salvar e quando liberar o editor.
Essa lógica era pequena, mas continha uma ordering constraint importante:
`releaseEditor` só pode ocorrer depois de um save bem-sucedido, de um descarte
explícito ou de um documento que já não esteja modificado.

`EditorSessionController` possui a materialização e a sincronização de
documentos, mas não deve conhecer diálogos da janela. Colocar a confirmação
dentro dele misturaria lifecycle de sessão com política de interação do shell.

## Decision

Criar `EditorTabCloseController`. O módulo:

- observa pedidos de fechamento do `QTabWidget`;
- ignora índices ou widgets que não são `CodeEditor`;
- pede uma decisão apenas para documentos modificados;
- não libera o editor em `Cancel`;
- libera imediatamente em `Discard`;
- chama o adapter de save e só libera depois de o documento deixar de estar
  modificado.

`MainWindow` fornece callbacks para a decisão visual, save e release. O
controller não cria diálogos, não escreve ficheiros e não possui a sessão de
editores; ele concentra apenas a política e a ordem das transições.

## Alternatives considered

### Manter a lambda em `MainWindow`

Rejeitado porque deixa a ordering constraint misturada com composição de
widgets e torna difícil testar cancelamento, descarte e falha de save sem
construir a janela.

### Colocar a política em `EditorSessionController`

Rejeitado porque a sessão deve coordenar documentos, enquanto a confirmação é
uma decisão de interação do shell. Isso faria o controller de sessão depender
de uma política visual que não é necessária para restauração ou sincronização.

### Fechar sempre e pedir confirmação no widget

Rejeitado porque perderia a capacidade de impedir o release quando o save é
cancelado ou falha.

## Consequences

O composition root continua dono dos adapters concretos, mas deixa de conhecer
a sequência de decisão do fechamento. O comportamento pode ser testado emitindo
o signal real do `QTabWidget`. A interface usa um enum pequeno para preservar
explicitamente os três resultados da confirmação.

## Verification

- `testEditorTabCloseControllerPreservesSaveOrdering` verifica cancelamento,
  descarte e save antes de release.
- O build CMake e o CTest passam após a extração.
