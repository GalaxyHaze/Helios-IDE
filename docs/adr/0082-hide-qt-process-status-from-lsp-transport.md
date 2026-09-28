# ADR 0082: Ocultar o status de QProcess da interface de transporte LSP

- Status: Accepted
- Date: 2026-09-27

`LspProcessTransport` já concentrava o lifecycle do processo, mas a sua
interface ainda emitia `QProcess::ExitStatus`; isso fazia `LspClient` conhecer
um detalhe Qt que deveria permanecer no adapter de transporte. A decisão é
publicar o value object `LspProcessResult`, com `exitCode` e `crashed`, e
traduzir `QProcess` apenas dentro da implementação do transporte.

## Considered Options

- **Manter `QProcess::ExitStatus` no signal**: rejeitado porque espalha o
  detalhe de framework para consumidores do transporte.
- **Publicar apenas `bool success`**: rejeitado porque o cliente precisa
  distinguir saída normal de crash para diagnóstico e apresentação de erro.
- **Publicar `LspProcessResult`**: escolhido porque mantém os dados relevantes
  pequenos e nomeados sem transportar a API de Qt através da seam.

## Consequences

- `LspClient.h` deixa de incluir e expor `QProcess`.
- O transporte continua livre para mudar o mecanismo de execução sem alterar
  o contrato do cliente.
- A decisão de “processo esperado” continua no `LspClient`; `crashed` descreve
  apenas o fato observado pelo adapter.
- O teste do transporte verifica o value object, não um enum de Qt.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `LspProcessTransport` e `LspProcessResult`
- `git diff --check`
