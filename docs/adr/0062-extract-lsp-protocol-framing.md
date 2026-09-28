# ADR 0062: Extrair o framing do protocolo LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`LspClient` acumulava três níveis diferentes de responsabilidade:

- gerir o processo do language server;
- manter requests pendentes, documentos abertos e capacidades negociadas;
- transformar bytes incrementais em mensagens JSON-RPC através de headers
  `Content-Length`.

O último nível tinha estado mutável próprio, limites de segurança e regras de
recuperação de headers inválidos. Mantê-lo dentro de `LspClient` tornava o
protocolo difícil de testar sem lançar um processo externo e misturava
transporte com política de requests e lifecycle.

## Decision

Criar `LspProtocolCodec` como módulo sem `QObject`, com duas operações:

- `encode`, que transforma um `QJsonObject` num frame LSP e rejeita mensagens
  acima do limite de 64 MB;
- `consume`, que aceita chunks arbitrários, mantém apenas o buffer incompleto,
  extrai uma ou várias mensagens completas e relata headers inválidos ou buffer
  excessivo através de um `DecodeResult`.

`LspClient` continua responsável por:

- criar, iniciar, parar e reiniciar o `QProcess`;
- escrever frames no processo;
- dispatch de responses, requests do servidor e notifications;
- requests LSP, cancelamento, timeouts, versionamento e estado dos documentos;
- sinais de domínio consumidos pelos controllers e painéis.

O codec não conhece `QProcess`, `LspClient`, URIs, versões, capabilities ou
signals da aplicação. O `LspClient` traduz erros do codec para `serverError` ou
`logMessage`, preservando a política de apresentação existente.

## Alternatives considered

### Manter o parser em `LspClient`

Rejeitado porque mantém estado de bytes junto ao lifecycle do processo e exige
testes com um servidor ou acesso a helpers privados para cobrir fragmentação.

### Criar um transport controller baseado em `QProcess`

Rejeitado neste estágio porque juntaria framing e lifecycle novamente. O ganho
de isolamento seria menor e a interface teria de expor detalhes do processo
sem uma segunda implementação que justificasse essa abstração.

### Criar um codec stateless que recebe o buffer inteiro

Rejeitado porque o protocolo é incremental por natureza. O seam precisa possuir
o buffer incompleto para que o chamador possa entregar chunks diretamente de
`readyReadStandardOutput`.

## Consequences

O framing pode ser testado com chunks partidos, múltiplos frames e headers
inválidos sem iniciar um servidor. `LspClient` fica focado na política LSP e
continua com a mesma interface pública. A recuperação de JSON inválido mantém
o comportamento anterior: o frame é descartado sem inventar um resultado de
domínio.

O codec ainda usa Qt Core (`QByteArray` e `QJsonObject`), mas não depende de
widgets nem de objetos de aplicação. Se outro transporte LSP for adicionado no
futuro, ele pode reutilizar o codec sem importar o lifecycle atual.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste de fragmentação, múltiplos frames e header inválido em
  `testLspProtocolCodecFramesIncrementally`
- `git diff --check`
