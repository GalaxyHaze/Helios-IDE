# ADR 0077: Extrair o transporte de processo LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` misturava duas camadas: a política de requests, documentos,
capabilities e dispatch de signals, e os detalhes de `QProcess`, framing
incremental, escrita de mensagens, stderr, falhas de arranque, terminação e
cleanup. Isso fazia mudanças no lifecycle do processo exigirem conhecimento
do cliente inteiro.

A decisão é introduzir `LspProcessTransport`. A seam inicia um executável,
escreve JSON-RPC como frames LSP, publica objetos decodificados, transporta
chunks de stderr e informa start, erro e término. `LspClient` continua
responsável pelo handshake, shutdown protocolar, requests pendentes,
coerência de versões, dispatch de notificações e sinais de domínio.

O transporte também mantém a distinção entre erros de decode, que continuam
a ser logados pelo cliente, input acima do limite, que é erro do servidor, e
falhas de escrita/encoding, que são erros de transporte.

## Consequences

- `LspClient` deixa de conhecer `QProcess`, `LspProtocolCodec` e reads de
  stdout/stderr.
- O lifecycle de processo e framing podem ser testados sem iniciar um
  `LspClient` nem negociar capabilities.
- A seam aceita apenas um processo ativo por vez e publica um único evento de
  término, evitando cleanup duplicado entre `errorOccurred` e `finished`.
- O protocolo de shutdown permanece no cliente porque depende do request
  `shutdown` e da política de restart pendente.
- Não foi criado um adapter genérico de processos: o módulo é específico ao
  transporte LSP e tem uma única implementação concreta.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de frame em `LspProcessTransport`
- `git diff --check`
