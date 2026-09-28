# ADR 0032: Isolar a projeção de estado do runtime LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` publicava o estado do runtime Zith, o diagnóstico de conexão e o
estado do clangd em três destinos: `StatusBarController`, `SettingsPanel` e
`LspManagerDialog`. As mesmas regras de conexão e sincronização apareciam
junto da política que inicia, para, reinicia e invalida os processos.

Essa mistura tornava fácil atualizar um destino e esquecer outro. Também fazia
o composition root conhecer a forma como um `LspClient` deve ser traduzido
para texto de diagnóstico.

## Decision

`LspRuntimePresentationController` owns only the projection of LSP state. It
offers four semantic operations:

- publicar o estado visual curto do LSP no status bar;
- atualizar runtime Zith e diagnóstico nos dois painéis;
- atualizar o estado descritivo do clangd nos dois painéis;
- atualizar apenas o diagnóstico quando muda a conexão.

The controller reads the existing runtime/client/coordinator state and accepts
callbacks for the two enablement settings and resolved clangd path. It does
not start or stop processes, choose restart policy, mutate documents, or own
any panel. `MainWindow` remains responsible for deciding when a refresh is
needed and for presenting transient messages/log lines.

## Alternatives considered

### Keep all projections in `MainWindow`

Would leave duplicated destination updates beside runtime lifecycle branches.
Every new presentation destination would enlarge the composition root and
increase the chance of inconsistent state.

### Put presentation into `ZithRuntimeLifecycleCoordinator`

Would make a process/lifecycle module depend on Settings, dialogs, and status
bar widgets. It would also make the runtime coordinator unusable without the
IDE shell.

### Create one presenter per destination

Would duplicate the connection/synchronization interpretation three times.
The shared controller keeps one interpretation and multiple presentation
adapters.

## Consequences

Runtime wording and connection-state interpretation have one implementation.
The lifecycle coordinators remain UI-independent, and future presentation
surfaces can be added without changing process policy. The controller still
depends on the existing shell widgets because the current product has one
presentation host; replacing that host remains a deliberate future seam.

## Verification

The build compiles both the application and test targets with the controller.
The complete CTest suite and static analysis are run after the extraction.
