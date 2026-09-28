# ADR 0069: Extrair o tracking de requests LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` mantinha diretamente o mapa de requests pendentes, o índice de
requests substituíveis por URI, timers de timeout e limpeza no cancelamento,
alteração de documento e encerramento do processo. A decisão é concentrar
esse ciclo de vida em `LspRequestTracker`; `LspClient` continua dono de
serializar/enviar JSON-RPC, traduzir cancelamentos para `$/cancelRequest` e
validar se uma resposta ainda corresponde à versão atual do documento.

O tracker expõe uma interface pequena para registar, retirar, cancelar por id,
cancelar por URI e limpar requests. Ele emite apenas o timeout; não conhece
processos, documentos ou widgets. A verificação URI/versão não foi movida
porque requer o estado documental do cliente e seria uma abstração artificial
separá-la do dispatch da resposta.

## Consequences

- Replacement, timeout e limpeza de requests têm uma única implementação.
- O transporte LSP não precisa conhecer índices internos de requests.
- A política de respostas obsoletas continua local ao `LspClient`, onde o
  estado de versões existe.
- O tracker pode ser testado sem iniciar um servidor LSP.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no tracker
- `git diff --check`
