# ADR 0013: Centralizar o lifecycle dos documentos LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` mantinha loops para reabrir documentos quando cada cliente LSP
ficava pronto e para desprender editores quando o servidor parava. A lógica era
duplicada para Zith e clangd e misturava uma invariável de documentos com
apresentação, status da janela e política de restart.

O risco mais importante era a assimetria entre os caminhos: o caminho Zith
selecionava documentos pela extensão, enquanto o caminho clangd também
confirmava que o editor estava associado ao cliente que emitira o evento.

## Decision

`LspEditorLifecycleController` é o módulo responsável por percorrer as tabs e
delegar o lifecycle de documentos para um cliente específico:

- abre apenas editores que estejam associados ao cliente, tenham um caminho
  nomeado; a política de linguagem, cliente, versão e envio pertence ao
  `LspDocumentCoordinator`;
- desprende apenas editores cujo cliente atual seja o cliente que parou,
  usando o mesmo coordinator que fecha documentos durante outras transições.

`MainWindow` continua responsável pelo estado visual, mensagens, diagnósticos e
política de restart, mas não conhece mais a mecânica de percorrer as tabs para
esta finalidade.

## Alternatives considered

### Manter os loops em `MainWindow`

É a menor alteração imediata, mas mantém duplicação e permite que os dois
clientes evoluam com invariantes diferentes.

### Fazer o próprio `LspDocumentCoordinator` percorrer as tabs

Isso misturaria a seleção de cliente por linguagem com ownership de widgets.
O coordinator continua útil para sincronizar um editor individual; o módulo de
lifecycle trata a coleção de editores e delega cada transição no seam existente.

### Extrair um controlador geral de UI/LSP

Seria uma interface superficial, com callbacks para quase toda a janela.
Preferimos uma seam menor, com uma invariável explícita e teste independente.

## Consequences

O lifecycle de documentos fica local e consistente para todos os clientes.
Adicionar outro cliente LSP requer conectá-lo ao módulo, sem duplicar loops no
shell da aplicação. A classe depende de `QTabWidget` e `CodeEditor`, portanto
não é um módulo puramente de domínio; essa dependência é deliberada porque a
invariável que ela protege é a associação entre uma tab visível e um cliente
LSP.

## Verification

O contrato de desprendimento seletivo é coberto por
`testLspEditorLifecycleDetachesOnlyOwnedEditors`.
