# ADR 0024: Isolar o ciclo de vida do runtime Zith

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` ligava diretamente `ZithToolchainManager` e `LspClient` e
misturava resolução de runtime, ativação do cliente, restart após crash,
estado de apresentação e tratamento de erros com wiring de widgets.

O shell também precisava expor o mesmo cliente a routers e ao
`WorkspaceCommandController`. Uma extração que escondesse completamente o
cliente quebraria essas integrações; uma extração que apenas movesse os
handlers para outra classe manteria uma interface rasa e callbacks
espalhados.

## Decision

`ZithRuntimeLifecycleCoordinator` torna-se o módulo responsável por:

- possuir o `LspClient` Zith e o `ZithToolchainManager`;
- resolver e ativar um runtime no workspace atual;
- manter `ZithRuntimeState`;
- aplicar `LspRestartPolicy` a encerramentos inesperados;
- controlar enable/disable, refresh, preferência online e limpeza de cache;
- publicar transições de estado, conexão, erro e eventos específicos do
  frontend.

A interface externa é pequena: o shell configura o workspace, habilita o
runtime, solicita resolução/refresh, altera a preferência online, limpa o
cache e obtém o cliente LSP como adapter para módulos que precisam falar
LSP. Uma alteração do workspace seguida de resolução força a comparação da
identidade completa do runtime, incluindo a raiz, antes de reutilizar o
processo. O coordenador não conhece widgets, panels, settings ou texto de
apresentação específico da janela.

`MainWindow` permanece como adapter de apresentação e composição:

- conecta routers e `WorkspaceCommandController` ao cliente exposto;
- traduz transições em status bar, settings, dialogs e logs;
- coordena o lifecycle visual dos documentos;
- mantém separado o lifecycle de clangd.

## Alternatives considered

### Manter toolchain e cliente no `MainWindow`

Evitaria uma nova seam, mas deixaria resolução, processo, restart e
apresentação acoplados ao shell. Cada alteração de runtime exigiria percorrer
handlers de constructor, settings, cache, crash e workspace.

### Esconder completamente o cliente LSP

Daria uma abstração mais fechada, mas obrigaria a duplicar ou reencaminhar
todos os sinais e métodos necessários aos routers, criando uma interface
maior do que o próprio cliente.

### Criar um coordenador genérico para todos os LSPs

Misturaria o runtime distribuído do Zith com a configuração local de clangd.
Os dois têm políticas de resolução, processo e configuração diferentes; a
separação mantém profundidade local e permite uma futura abstração apenas se
houver um segundo adapter real.

## Consequences

O shell deixou de possuir diretamente o toolchain, o estado do runtime e a
política de restart. Falhas de resolução, cache e restart têm agora um único
local de implementação e uma interface testável.

O cliente LSP continua visível como adapter por necessidade real. Isso mantém
o custo da integração atual baixo, mas deixa explícito que o próximo
aprofundamento possível é uma interface de eventos LSP, caso surja um segundo
consumidor que justifique essa seam.

## Verification

`testZithRuntimeLifecycleCoordinatorOwnsDisableAndCacheTransitions` verifica
enable/disable, publicação de estado, acesso ao cliente e remoção do cache
através da interface do coordenador.
