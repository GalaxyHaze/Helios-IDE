# ADR 0073: Extrair o decoding de resultados LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` acumulava duas responsabilidades diferentes: transportar
JSON-RPC através do processo LSP e interpretar as várias formas de resultados
do protocolo. Completion podia ser array ou `CompletionList`, locations podiam
ser objeto, array ou `LocationLink`, hover podia conter string, markup object
ou múltiplos conteúdos, e notificações precisavam ser convertidas em
diagnostics.

A decisão é colocar essas conversões em `LspResultDecoder`. O módulo recebe
JSON LSP e devolve os value objects já usados pelo editor. Não conhece
processos, requests pendentes, versões documentais ou signals. `LspClient`
continua dono do transporte, da validação de respostas stale e da publicação
de signals.

## Consequences

- Variantes do protocolo ficam localizadas num único módulo.
- O cliente LSP fica mais focado em lifecycle, transporte e request policy.
- O decoder pode ser testado com JSON estático, sem iniciar um servidor.
- A representação interna dos value objects continua em `LspClient.h`, evitando
  uma segunda hierarquia de tipos.
- O módulo deliberadamente não interpreta capability negotiation nem políticas
  de erro JSON-RPC; essas decisões continuam no cliente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no decoder
- `git diff --check`
