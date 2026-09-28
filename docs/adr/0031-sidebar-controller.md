# ADR 0031: Isolar a política de apresentação da sidebar

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` interpretava diretamente os eventos da `ActivityBar` e mantinha a
política de apresentação da sidebar. Essa política incluía alternar a
sidebar quando o modo já ativo era clicado, mudar o índice do stack, atualizar
o Git ao selecionar esse modo, persistir a visibilidade e preparar os dados
do modo Settings.

Os painéis individuais não devem conhecer essa política, e mover apenas os
ponteiros dos widgets para outra classe não criaria uma seam útil.

## Decision

`SidebarController` owns the sidebar presentation policy. Its small interface
offers semantic operations:

- `selectMode`, which implements selection-versus-toggle behavior;
- `setVisible`, which persists visibility and synchronizes the activity bar;
- `showSettings`, which prepares Settings before making it visible;
- `synchronizeActivityBar`, for restoring shell state after Qt restores dock
  state.

The controller connects `ActivityBar::modeChanged` itself. The host supplies
only the persistence and Settings-preparation callbacks. `GitPanel` remains
the owner of Git operations; the controller only asks it to refresh when the
Git mode becomes active.

## Alternatives considered

### Keep mode policy in `MainWindow`

Would leave shell composition coupled to toggle rules, persistence, and panel
activation. Each new activity mode would increase the composition root's
knowledge.

### Make `ActivityBar` own the sidebar

Would couple a navigation widget to the panel stack, settings persistence, and
Git behavior. The visual widget would become a second composition root.

### Create a widget registry or getter-only shell module

Would move construction without hiding behavior. Callers would still need to
know the stack indices and the same visibility rules, producing a shallow
interface.

## Consequences

Sidebar behavior has one focused test surface and the shell no longer handles
activity-mode signal wiring. Adding a new mode still requires extending the
activity enum and panel stack, but its selection and visibility semantics
remain localized.

The controller intentionally does not own panel contents, workspace roots, or
settings values. Those remain domain and host responsibilities.

## Verification

`testSidebarControllerKeepsModeAndVisibilityPolicy` verifies mode selection,
toggle-off behavior, Settings preparation, activity-bar synchronization, and
the host visibility callback. The complete build and test suite must pass
after the extraction.
