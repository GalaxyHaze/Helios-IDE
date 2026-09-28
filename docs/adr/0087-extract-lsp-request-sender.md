# ADR 0087: Extrair o sender de requests LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` acumulava a política mecânica de envio de requests com a política
de cada capacidade LSP. O cliente atribuía IDs, criava envelopes JSON-RPC,
registava timeouts, substituía requests canceláveis, cancelava por URI,
enviava `$/cancelRequest` e limpava o tracker quando o processo terminava.
Essas invariantes eram independentes do decoder e dos sinais específicos de
completion, hover ou navigation.

A decisão é introduzir `LspRequestSender`. O módulo recebe callbacks pequenos
para escrita e estado do processo, e expõe envio, conclusão, cancelamento por
ID, cancelamento por URI e limpeza. O `LspClient` continua a construir a
intenção de cada request através da sua interface privada existente, validar
versões documentais, tratar erros JSON-RPC, decodificar resultados e emitir
signals de aplicação.

## Considered Options

- **Manter IDs e tracker no `LspClient`**: rejeitado porque cada alteração de
  timeout ou replacement continuaria a atravessar lifecycle e handlers de
  resultados.
- **Extrair um builder de JSON apenas**: rejeitado porque esconderia apenas
  sintaxe, mas deixaria no cliente as invariantes de cancellation, timeout e
  cleanup.
- **Extrair `LspRequestSender`**: escolhido porque concentra o ciclo mecânico
  completo de um request atrás de uma interface pequena e injetável.

## Consequences

- `LspClient` deixa de conhecer o contador de IDs e o `LspRequestTracker`
  diretamente.
- Replacement, timeout e cancelamento por URI têm uma única implementação e
  podem ser testados sem processo LSP.
- O sender não conhece `LspDocumentRegistry` nem decide se uma resposta está
  stale; essa decisão permanece junto do dispatch da resposta.
- O sender depende de callbacks de escrita/execução para não introduzir um
  segundo adapter de processo nem acoplar testes a `QProcess`.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `LspRequestSender`
- `clang-tidy` focado no sender
- `git diff --check`
