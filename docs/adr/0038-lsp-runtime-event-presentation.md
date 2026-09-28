# ADR 0038: Concentrar a projeção de eventos do runtime LSP

- Status: Accepted
- Date: 2026-09-27

## Context

Além de publicar o estado materializado do runtime, `MainWindow` interpretava
os eventos `frontendStatusChanged` e `metricsReceived`. O status frontend era
convertido em texto e cor para a barra de estado, enquanto os logs eram
duplicados manualmente em `SettingsPanel` e `LspManagerDialog`.

Isso deixava a interpretação de eventos ao lado da composição da janela e
permitia que um novo destino recebesse apenas parte do telemetry.

## Decision

`LspRuntimePresentationController` passa a expor operações semânticas para
projetar status frontend e métricas. O módulo:

- traduz estados `warming`, `ready` e `error` para a apresentação LSP;
- publica o log de status nos dois destinos existentes;
- serializa métricas compactamente e publica o mesmo registro nos dois
  destinos;
- continua sem iniciar, parar ou reiniciar processos.

As conexões dos sinais do runtime são feitas diretamente para o controller
depois de sua construção. `MainWindow` permanece responsável por compor os
objetos e por decidir outras transições de lifecycle, mas não interpreta esses
eventos.

## Alternatives considered

### Manter os slots em `MainWindow`

Preservaria pouca infraestrutura nova, mas manteria a duplicação de destino e
espalharia a semântica de telemetry pelo composition root.

### Criar um controller separado para cada evento

Seria uma decomposição rasa: status e métricas compartilham os mesmos destinos,
o mesmo lifecycle de apresentação e a mesma regra de publicação.

### Colocar os logs no runtime lifecycle

Faria um módulo de processo conhecer widgets de Settings e do LSP Manager,
invertendo a separação entre lifecycle e apresentação.

## Consequences

A interface de apresentação do runtime cobre estado materializado e eventos
transitórios no mesmo seam. A interpretação de status e a duplicação de logs
ficam locais, enquanto novos destinos podem ser adicionados sem alterar o
runtime lifecycle. O controller continua Qt-facing porque a apresentação
atual é composta por widgets concretos.

## Verification

`testLspRuntimePresentationProjectsFrontendEvents` verifica a tradução de
status frontend para o status bar e a publicação de métricas nos dois logs.
