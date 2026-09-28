# ADR 0098: Extrair o protocolo de documentos LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` acumulava a coordenação do registry de versões, a serialização das
notificações `didOpen`/`didChange`/`didSave`/`didClose` e o cancelamento de
requests associados a uma URI. Essa combinação fazia o cliente conhecer
detalhes de protocolo e permitia que a ordem entre invalidar requests,
atualizar a versão e enviar a notificação se espalhasse por futuros callers.

A decisão é criar o módulo `LspDocumentProtocol`. A sua interface recebe
somente três dependências semânticas: saber se a sessão está pronta, escrever
uma mensagem JSON e cancelar requests de uma URI. O módulo mantém o registry
de versões e produz as notificações LSP, preservando a política existente:

- operações de abertura, alteração e save não têm efeito quando a sessão não
  está pronta;
- alterações cancelam requests da URI antes de enviar `didChange`;
- close cancela requests e remove a URI do registry mesmo quando a sessão já
  não está pronta;
- respostas associadas a uma versão antiga são rejeitadas pelo mesmo registry.

## Considered Options

- **Manter toda a lógica no `LspClient`**: rejeitado porque o cliente
  continuaria a misturar processo, requests, protocolo de documentos e
  capabilities.
- **Extrair apenas builders JSON**: rejeitado porque moveria serialização sem
  mover os invariantes de prontidão, versionamento e cancelamento.
- **Criar um adapter por notificação**: rejeitado porque produziria módulos
  rasos e multiplicaria seams sem comportamento próprio.
- **Extrair um módulo de protocolo de documentos**: escolhido porque concentra
  uma política observável, tem uma interface pequena e pode ser testado sem
  iniciar um processo LSP.

## Consequences

- `LspClient` continua a ser a fachada pública para os controllers existentes,
  mas deixa de possuir diretamente o registry de documentos.
- A ordem de cancelamento e atualização fica local ao módulo e é coberta por
  teste unitário.
- O módulo ainda usa JSON porque a seam está entre o domínio do documento e o
  transporte LSP; um futuro transporte diferente pode ser introduzido sem
  reabrir os callers do editor.
- O processo, o lifecycle de shutdown e o request lifecycle permanecem no
  `LspClient`/`LspRequestSender`; esta decisão não tenta criar uma abstração
  genérica para cada método do cliente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cppcheck` e `clang-tidy` no novo módulo; os únicos resultados são os
  avisos globais já existentes de Qt/MOC e convenções `modernize-*`.
