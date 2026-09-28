# ADR 0138: Fonte de eventos LSP para consumidores da shell

- Status: Accepted
- Date: 2026-09-28

Consumidores da shell precisavam ligar diretamente sinais de `LspClient` para
diagnostics, mensagens, lifecycle, output de tarefas, progresso, resultados de
comandos e pedidos de save-all. Isso expunha uma superfície de protocolo muito
maior do que a necessária e tornava os testes dependentes da forma concreta do
cliente, mesmo quando não precisavam iniciar um processo.

A decisão é introduzir `LspEventSource` como uma interface pequena de eventos
de shell e `LspClientEventSource` como o adapter que traduz os sinais do
cliente concreto. `LanguageServiceFeedbackController`,
`WorkspaceTaskOutputController`, `WorkspaceCommandController` e
`LspRuntimeEventController` consomem a fonte; continuam a receber
`LspClient` separadamente apenas quando precisam executar comandos ou coordenar
documentos.

Resultados de features do editor, requests client-initiated, sincronização de
documentos e controle de processo permanecem fora da interface. Esses eventos
variam por router e não formam um conjunto compartilhado pela shell.

## Alternativas

- Ligar todos os consumidores diretamente a `LspClient`: rejeitado porque
  espalhava conhecimento de sinais de protocolo e dificultava fontes fake.
- Criar um event bus genérico: rejeitado porque perderia locality, ordenação e
  ownership dos eventos, além de aumentar a superfície pública.
- Criar uma interface para todos os métodos e resultados de `LspClient`:
  rejeitado porque seria uma duplicação rasa do cliente e misturaria requests,
  lifecycle e apresentação.
- Manter apenas um adapter sem fonte fake: rejeitado porque o seam não teria
  uma segunda implementação verificável.

## Consequências

- Consumidores de shell podem ser testados emitindo eventos numa fonte fake,
  sem lançar um language service.
- A tradução de sinais concretos fica localizada em `LspClientEventSource`.
- O composition root declara separadamente o cliente usado para comandos e a
  fonte usada para eventos.
- Routers de features continuam com `LspClient` enquanto não houver um
  conjunto de eventos compartilhado que justifique outro seam.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
