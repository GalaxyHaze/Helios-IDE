# ADR 0076: Agrupar as capabilities negociadas do servidor LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` mantinha quinze flags de providers e o `textDocumentSync` como
estado paralelo. O parsing de capabilities também vivia no cliente, embora
seja uma conversão de dados da resposta `initialize`, independente de
processo, requests ou signals.

A decisão é representar esse conjunto como `LspServerCapabilities`. O value
object oferece `fromJson`, preserva os defaults atuais e fica armazenado como
uma unidade no cliente. Os getters existentes de `LspClient` continuam
disponíveis para evitar espalhar a negociação protocolar pelos callers.

## Consequences

- A resposta de `initialize` tem uma representação nomeada e testável.
- O estado do cliente deixa de ter um grupo paralelo de flags soltas.
- O `LspClient` continua sendo a seam de lifecycle e transporte; capabilities
  não ganham sinais nem política de UI.
- O `documentSyncKind` permanece disponível no cliente, mas a sua origem e os
  defaults ficam centralizados no value object.
- Não foi criado um registry genérico de features: a lista corresponde ao
  contrato efetivamente consumido por Helios e pode crescer junto com o
  protocolo.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
