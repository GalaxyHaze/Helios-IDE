# ADR 0053: Compor dependências de UI antes dos controllers que as consomem

- Status: Accepted
- Date: 2026-09-27

## Context

O `MainWindow` cria vários controllers durante a composição. Um controller
pode receber ponteiros para painéis que ainda não foram compostos; nesse caso
o valor `nullptr` é capturado no construtor e a dependência permanece ausente
durante toda a vida do controller. Isso aconteceu com
`LspRuntimeController` e `DiagnosticsPanel`: a rotina de desativação do LSP
pretendia limpar os diagnósticos, mas não conseguia fazê-lo porque o painel
ainda não existia quando o controller foi criado.

## Decision

O composition root deve criar cada widget que representa uma dependência
antes de criar controllers que possam ler ou mutar esse widget. A criação do
`BottomPanel` e a aquisição dos seus subpainéis (`DiagnosticsPanel`,
`CompilerPanel` e `ReferencesPanel`) acontecem antes da criação de
`LspRuntimeController`. O docking e a apresentação do painel continuam sendo
configurados posteriormente, no ponto em que a janela termina de compor a
infraestrutura visual.

Essa regra é específica para dependências concretas: não será criado um
framework genérico de ordenação nem um setter tardio apenas para contornar
uma composição incompleta.

## Alternatives considered

### Manter a ordem e adicionar setters tardios

Rejeitado porque permite que o controller opere em estado parcialmente
composto e torna a validade da interface dependente de uma sequência implícita
de chamadas.

### Fazer o controller procurar o painel depois

Rejeitado porque espalharia conhecimento do composition root pelo módulo de
runtime e esconderia uma dependência obrigatória atrás de lookup de objetos.

### Criar todo o shell antes de todos os controllers

Rejeitado porque seria uma regra ampla demais: controllers que só dependem de
serviços ou de outros controllers não precisam esperar pela composição visual
completa. A ordem deve refletir as dependências reais.

## Consequences

O controller recebe uma interface válida desde a construção e a transição de
desativação pode limpar diagnósticos de forma determinística. Alterações
futuras no construtor de `MainWindow` devem preservar a ordem de dependências,
ou alterar a interface para tornar explicitamente opcional uma dependência.

## Verification

`testLspRuntimeControllerOwnsDisableTransition` agora injeta diagnósticos,
desativa o runtime e verifica que o painel foi limpo. A mesma verificação
protege o contrato que motivou a correção da composição.
