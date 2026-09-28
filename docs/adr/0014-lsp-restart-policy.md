# ADR 0014: Isolar a política de restart do runtime LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` mantinha timestamps de falhas, a janela temporal de sessenta
segundos, o limite de três reinícios e o cálculo do backoff exponencial do
`zith-lsp`. Essa política estava misturada com atualização de labels, status da
janela e agendamento do processo.

Além de aumentar o estado do shell da aplicação, essa forma tornava difícil
verificar cenários importantes: paragem esperada, runtime desativado, repetição
rápida de crashes e recuperação depois da janela temporal.

## Decision

`LspRestartPolicy` passa a ser o módulo responsável por decidir a reação a uma
paragem inesperada. Dado o instante, as condições de execução e o facto de a
paragem ser esperada, devolve uma decisão:

- `Ignore` quando não deve haver restart;
- `Restart` com número da tentativa e atraso de `1`, `2` ou `4` segundos;
- `GiveUp` quando já ocorreram três tentativas dentro da janela de sessenta
  segundos.

`MainWindow` aplica a decisão: apresenta o estado, atualiza a informação de
runtime e usa `QTimer` para iniciar o processo. A política não conhece Qt
widgets, mensagens ou detalhes de processo.

## Alternatives considered

### Manter a política no `MainWindow`

Evita um ficheiro novo, mas mantém estado temporal e regras de recuperação
misturados com a composição da janela.

### Fazer `ClangdLifecycleCoordinator` possuir esta política

O coordenador de clangd trata reconciliação de configuração e estado do
servidor C-family. O restart automático aqui é específico do runtime Zith e
tem uma política temporal diferente; juntá-los confundiria os dois domínios.

### Usar apenas um contador de crashes

Não preservaria a proteção contra loops de restart ao longo do tempo. A janela
temporal é parte da política, não apenas um detalhe de apresentação.

## Consequences

A política pode ser testada como valor sem iniciar um processo LSP. A janela
fica responsável apenas por efeitos externos e apresentação. Se futuramente a
estratégia de recuperação mudar, o cálculo temporal permanece localizado, mas
as mensagens e o mecanismo de agendamento não precisam ser alterados juntos.

## Verification

`testLspRestartPolicyUsesBoundedExponentialBackoff` cobre paragens ignoradas,
backoff, limite de tentativas e recuperação após a janela temporal.
