# ADR 0070: Extrair o builder de inicialização LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient::onProcessStarted` construía diretamente o payload completo de
`initialize`, misturando capacidades comuns, `rootUri/rootPath`, opções da
stdlib e extensões experimentais específicas do Zith. A decisão é colocar
essa composição em `LspInitializationBuilder`, uma interface pura que recebe
um `LspInitializationOptions` nomeado e retorna os parâmetros JSON;
`LspClient` continua dono de iniciar o processo, enviar o request e reagir à
resposta.

O builder mantém uma única matriz de capacidades para os dois servidores e
explicita o que só pertence ao Zith. A compatibilidade de `LspStartOptions`
não muda nesta tranche; o builder é uma seam interna do transporte de
lifecycle, não uma nova configuração pública para os callers.

## Consequences

- O contrato de inicialização pode ser testado sem executar clangd ou Zith.
- Diferenças entre servidores ficam localizadas e deixam de ocupar o
  lifecycle do processo.
- A validação de capabilities retornadas continua no `LspClient`, pois é
  estado materializado da sessão.
- Alterações futuras no protocolo de inicialização têm um único ponto de
  composição.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no builder
- `git diff --check`
