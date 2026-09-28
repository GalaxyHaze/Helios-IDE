# ADR 0045: Isolar a projeção de feedback dos language services

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` recebia diagnostics de dois clientes LSP, encaminhava-os para o
painel, projetava as contagens na status bar e tratava mensagens e resultados
de rename. Embora cada conexão fosse curta, a combinação codificava uma
invariante importante: os dois language services devem produzir o mesmo tipo de
feedback nas mesmas superfícies da aplicação.

Manter essa política no composition root fazia a janela conhecer detalhes dos
signals de cada cliente e tornava fácil adicionar um terceiro cliente com
comportamento incompleto. Um router genérico de todos os resultados LSP seria
mais amplo do que a responsabilidade real; o problema aqui é feedback de
language service, não routing de cada capacidade do protocolo.

## Decision

Criar `LanguageServiceFeedbackController` como módulo de projeção. Ele:

- recebe diagnostics de qualquer `LspClient` anexado e atualiza o
  `DiagnosticsPanel`;
- observa a mudança de contagens do painel e publica os valores através de um
  callback semântico;
- encaminha resultados de rename como workspace edits;
- transforma mensagens de servidor em mensagens de status com timeout de cinco
  segundos.

O módulo possui uma interface pequena: o composition root fornece o painel,
callbacks de aplicação e apresentação, e chama `attach` para cada cliente.
`MainWindow` continua dono da composição, da aplicação concreta do
`WorkspaceEdit` e da apresentação da status bar; o controller possui a
invariante de que todos os clientes anexados seguem a mesma política.

## Alternatives considered

### Manter as conexões em `MainWindow`

Rejeitado porque mantém duplicação entre clientes no composition root e torna
o comportamento de um novo language service dependente de novas lambdas.

### Criar um router genérico de resultados LSP

Rejeitado porque misturaria diagnostics, rename, completion, referências e
resultados específicos do editor. A interface seria maior e mais superficial,
sem uma política única além de encaminhar signals.

### Fazer o `LspClient` conhecer a status bar e os painéis

Rejeitado porque acoplaria o transporte/protocolo à apresentação da aplicação e
impediria reutilizar o cliente fora desta janela.

## Consequences

`MainWindow` deixa de conhecer a wiring de diagnostics, rename e mensagens dos
clientes. A política comum fica testável sem construir a janela inteira. O
controller ainda usa adapters concretos para o painel e callbacks para as
superfícies da janela; ele não é dono do estado do processo LSP nem do
workspace edit.

## Verification

- `testLanguageServiceFeedbackControllerProjectsClientEvents` verifica
  diagnostics, contagens, rename e mensagens.
- O build CMake e o CTest devem ser executados após a extração.
