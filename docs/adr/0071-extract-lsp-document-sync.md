# ADR 0071: Extrair a política de sincronização documental LSP

- Status: Accepted
- Date: 2026-09-27

`CodeEditor` precisava manter o snapshot anterior do texto, converter eventos
`QTextDocument` em ranges LSP, acumular alterações durante o debounce,
incrementar a versão e escolher entre sincronização incremental e full. Essa
política estava misturada com rendering, input e apresentação do editor.

A decisão é colocá-la em `LspDocumentSync`. O módulo recebe eventos de mudança
e o texto corrente, conserva o snapshot e produz um `LspDocumentSyncBatch`
versionado. O batch informa se deve ser enviado como alterações incrementais
ou como o documento completo, de acordo com o `syncKind` anunciado pelo
servidor.

`CodeEditor` continua dono do `QTextDocument`, do timer de debounce e do
`LspClient`. Ele apenas traduz o evento Qt para o value object
`LspDocumentChangeInput`, envia o batch produzido e não expõe os detalhes do
snapshot. A seam não conhece widgets nem inicia efeitos de rede.

## Consequences

- O cálculo de posições e o versionamento deixam de ser duplicados no widget.
- O comportamento incremental/full pode ser testado sem criar um editor Qt.
- O editor mantém a responsabilidade de decidir quando sincronizar e para
  qual cliente enviar.
- Alterações futuras no protocolo de sincronização ficam localizadas no módulo.
- O módulo depende dos value objects LSP existentes, mas não adiciona uma
  segunda representação de documento ao domínio.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no módulo
- `git diff --check`
