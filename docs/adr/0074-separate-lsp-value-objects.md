# ADR 0074: Separar os value objects LSP do cliente

- Status: Accepted
- Date: 2026-09-27

Os value objects LSP estavam declarados em `LspClient.h`. Isso fazia módulos
que apenas trabalhavam com posições, ranges, diagnostics ou resultados JSON
dependerem também do header que expõe processo, requests, signals e lifecycle.

A decisão é mover esses tipos para `LspTypes.h`. `LspClient.h` continua a
incluí-lo para preservar a interface pública existente, mas `LspResultDecoder`
e `LspDocumentSync` passam a depender apenas dos tipos que realmente usam.

## Consequences

- O seam de dados do protocolo fica independente do transporte.
- Headers de módulos puros deixam de puxar o cliente LSP completo.
- Callers existentes não precisam mudar os nomes dos tipos nem os includes
  imediatos.
- Novos value objects devem ser adicionados aqui apenas quando forem
  compartilhados por mais de uma política LSP; tipos específicos de uma
  implementação permanecem locais.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
