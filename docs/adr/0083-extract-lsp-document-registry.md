# ADR 0083: Extrair o registry de documentos LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` mantinha diretamente o mapa de documentos e repetia a regra de
validar respostas e diagnósticos contra a versão corrente. A decisão é
introduzir `LspDocumentRegistry`, um módulo de estado pequeno que centraliza
abertura, atualização, fecho, consulta de versão e aceitação de resultados
versionados; o cliente continua dono do envio JSON e do cancelamento de
requests.

## Considered Options

- **Manter o `QHash` no cliente**: rejeitado porque deixa a invariável de
  stale-result espalhada por handlers de resposta e notificação.
- **Misturar o registry com `LspDocumentSync`**: rejeitado porque sync agrega
  alterações textuais do editor, enquanto o registry representa a visão de
  versões conhecida pelo cliente LSP.
- **Extrair `LspDocumentRegistry`**: escolhido porque concentra a invariável
  documental e fornece uma interface pura e diretamente testável.

## Consequences

- `LspClient` deixa de conhecer a representação do mapa de documentos.
- Respostas e diagnósticos usam a mesma decisão de versão corrente.
- Notificações sem versão continuam aceites, preservando a semântica do
  protocolo atual.
- O registry não cancela requests nem envia mensagens; essas responsabilidades
  permanecem no cliente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `LspDocumentRegistry`
- `git diff --check`
