# ADR 0018: Separar o routing de completion LSP

- Status: Accepted
- Date: 2026-09-27

## Context

O construtor de `MainWindow` recebia resultados de completion de dois clientes
LSP e fazia três decisões: ignorar resultados quando o LSP estava desativado,
aceitar apenas a URI da tab ativa e combinar itens do servidor com snippets
locais antes de abrir o completer.

Essa política era independente da composição de menus, referências,
workspace edits e status do runtime, mas estava no mesmo método de wiring.

## Decision

`LspCompletionRouter` recebe tabs, snippet manager, completer, modelo e uma
função pequena que informa se o LSP está habilitado. Para cada cliente
associado:

- descarta resultados quando o LSP está desabilitado;
- descarta resultados de URI que não corresponde à tab ativa;
- combina itens LSP e snippets;
- atualiza o modelo e abre o completer quando existe conteúdo.

O shell continua a limpar o modelo durante transições de runtime; o router
não possui política de lifecycle nem de persistência.

## Alternatives considered

### Manter o lambda em `MainWindow`

Era curto, mas duplicava o mesmo contrato para cada cliente e misturava
completion com o restante do lifecycle da janela.

### Colocar snippets dentro de `LspCompletionModel`

Isso faria o modelo conhecer uma fonte de dados específica da aplicação. O
router é o lugar da composição entre resultado remoto e snippets locais.

### Criar um router para todas as respostas LSP

Misturaria completion com resultados document-bound, references, menus e
workspace edits. Essas políticas têm invariantes diferentes e permanecem
seams separados.

## Consequences

O resultado de completion tem uma única política de tab ativa e composição de
snippets. A função `isEnabled` é a única dependência de estado do shell e
mantém o módulo testável sem conhecer settings ou status bar.

## Verification

`testLspCompletionRouterCombinesOnlyForActiveEditor` verifica routing para a
tab ativa e descarte de URI estrangeira.
