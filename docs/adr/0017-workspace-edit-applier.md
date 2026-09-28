# ADR 0017: Separar a aplicação workspace-aware de WorkspaceEdit

- Status: Accepted
- Date: 2026-09-27

## Context

`WorkspaceEdit` já era responsável por interpretar o formato LSP e validar
ranges sobre texto. `MainWindow`, porém, ainda decidia como encontrar uma tab
aberta, ler targets fechados, validar todos os targets antes de mutar e
persistir alterações em disco.

Isso fazia o shell da aplicação conhecer detalhes de edição de documentos e
também tornava difícil garantir que uma resposta inválida não alterasse parte
do workspace.

## Decision

`WorkspaceEditApplier` recebe as tabs abertas e expõe `apply`, devolvendo um
`Result` com `applied` e `error`. A implementação tem duas fases:

1. parseia e prepara todos os targets, usando o texto vivo para editors abertos
   e o conteúdo do ficheiro para targets fechados;
2. só depois da validação completa aplica edits em editors ou faz commit com
   `QSaveFile`.

O módulo não apresenta mensagens. `MainWindow` traduz o erro para status bar,
mantendo apresentação fora da seam.

## Alternatives considered

### Manter a aplicação no `MainWindow`

Preserva menos classes, mas mistura workspace mutation com composição de UI e
deixa o shell responsável por invariantes de atomicidade de preparação.

### Expandir `WorkspaceEdit`

Isso faria o parser de protocolo conhecer tabs, widgets e filesystem. O
parser continua puro; o applier é o módulo workspace-aware que o consome.

### Aplicar target a target sem fase de preparação

Seria mais curto, mas permitiria mutação parcial quando um target posterior
falha na leitura ou contém range inválido. A preparação completa é um
trade-off deliberado por consistência.

## Consequences

Rename e code actions podem aplicar edits sem duplicar lookup de tabs ou
tratamento de ficheiros. O applier depende de `QTabWidget` e `CodeEditor`,
porque a diferença entre documento aberto e fechado é parte do comportamento
workspace-aware. A apresentação de erros continua substituível no shell.

## Verification

`testWorkspaceEditApplierUpdatesOpenAndClosedTargets` verifica a aplicação
coordenada em um editor aberto e um ficheiro fechado.
