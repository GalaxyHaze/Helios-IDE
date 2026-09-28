# ADR 0100: Centralizar o dispatch de respostas LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` recebia cada resposta JSON, consultava diretamente o
`LspRequestTracker`, verificava a versão do documento, filtrava os códigos de
cancelamento e executava o callback pendente. Isso fazia a fachada do cliente
conhecer os detalhes do request lifecycle apesar de `LspRequestSender` já
possuir o estado necessário para tomar essas decisões.

A decisão é mover esse comportamento para
`LspRequestSender::handleResponse`. O sender consome o pending request,
consulta uma função semântica `isCurrentDocument(uri, version)`, ignora
respostas obsoletas, suprime os erros de cancelamento esperados e encaminha
callbacks ou logs de erro. O `LspClient` continua responsável por distinguir
responses de notifications e fornece apenas a regra de freshness documental.

## Considered Options

- **Manter `handleResponse` no `LspClient`**: rejeitado porque espalha o
  request lifecycle entre a fachada e o sender.
- **Criar um decoder separado para respostas**: rejeitado porque decoding de
  payload e ownership de pending requests são preocupações diferentes; o
  decoder não deveria consumir o tracker.
- **Fazer o sender apenas retornar o pending request**: rejeitado porque
  manteria a filtragem de freshness, erro e callback no caller, preservando o
  acoplamento original.
- **Centralizar dispatch no sender e injetar freshness**: escolhido porque
  concentra a política de lifecycle sem fazer o sender conhecer o editor ou o
  registry concreto de documentos.

## Consequences

- A interface do `LspClient` deixa de ter `handleResponse` privado e o sender
  deixa de expor `take()` como detalhe de implementação.
- O sender tem leverage sobre todos os requests: resposta válida, stale,
  cancelada e com erro seguem a mesma política.
- A regra de freshness continua fora do sender, preservando a separação entre
  request lifecycle e estado documental.
- Callbacks ainda recebem o JSON bruto para que os métodos de feature usem os
  decoders especializados existentes.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- Teste `testLspRequestSender` para resposta válida, stale e erro.
- `git diff --check`
