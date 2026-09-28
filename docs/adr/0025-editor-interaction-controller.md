# ADR 0025: Isolar a política de interação do editor

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` ainda ligava sinais de cada `CodeEditor` diretamente a ações LSP,
navegação, zoom, chrome e comandos Vim. O mesmo shell também interpretava
`w`, `q`, `q!` e `wq`, decidindo quando salvar, recusar o fechamento ou
libertar o editor.

`EditorSessionController` já possui a política de materialização, abertura,
sincronização e restauração de tabs. Adicionar comandos Vim a esse módulo
misturaria lifecycle de documentos com intenção de interação do usuário.

## Decision

`EditorInteractionController` owns the editor interaction policy:

- attaches editor signals to LSP actions and location navigation;
- synchronizes cursor changes with editor chrome;
- applies editor zoom changes to the appearance policy;
- publishes the active Vim mode label;
- interprets `w`, `q`, `q!`, and `wq`;
- decides when a modified document must refuse `q`;
- delegates save, close, status presentation, and central-widget updates
  through a small callback interface.

`EditorSessionController` remains responsible for tab and document lifecycle.
`MainWindow` supplies the callbacks and remains the composition root; it no
longer parses Vim commands or wires these editor signals itself.

## Alternatives considered

### Keep interaction handling in `MainWindow`

Would avoid a module, but the shell would continue to own editor event wiring,
command grammar, and close/save policy alongside workspace, runtime, and
presentation concerns.

### Add Vim commands to `EditorSessionController`

Would reduce one class, but make a document-session module depend on user
commands, status presentation, and save/close intent. The resulting interface
would be wider and less coherent.

### Create one controller per signal

Would produce shallow modules that each forward one widget signal. The
interaction controller keeps the coherent policy cluster together and exposes
one attach seam plus one command seam.

## Consequences

New editor commands and interaction rules have one implementation and one
test surface. The session module has fewer presentation callbacks, and the
shell loses the Vim grammar.

The controller still receives callbacks for save and close because those
operations belong to the active window/session. If another host needs the
same interaction policy, those callbacks are the seam to replace; no widget
ownership is transferred implicitly.

## Verification

`testEditorInteractionControllerInterpretsVimCommands` verifies mode-label
updates, refusal to close modified documents, save-before-close for `wq`, and
central-widget refresh through the controller interface.
