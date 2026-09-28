# ADR 0016: Centralizar o routing de resultados ligados ao documento

- Status: Accepted
- Date: 2026-09-27

## Context

Os resultados de formatting e document symbols eram ligados diretamente no
construtor de `MainWindow`. Cada handler repetia a mesma regra: só aplicar o
resultado ao editor atual quando URI e versão ainda correspondessem.

Essa regra protege contra respostas assíncronas atrasadas. Sem ela, uma
resposta legítima de uma versão anterior poderia formatar o documento errado
ou substituir o outline de outra tab.

## Decision

`LspEditorResultRouter` recebe as tabs e o outline, liga-se a cada cliente LSP
e centraliza o routing de resultados document-bound:

- formatting só é aplicado ao editor atual com URI e document version iguais;
- document symbols só atualizam o outline quando o editor atual ainda
  corresponde à resposta;
- respostas sem editor atual, com URI diferente ou versão antiga são
  descartadas.

Resultados com política própria, como completion, code actions, references,
mensagens e workspace edits, permanecem em módulos distintos até terem uma
seam adequada.

## Alternatives considered

### Repetir guards em `MainWindow`

Funciona, mas multiplica uma invariável assíncrona importante e torna fácil
esquecer a verificação de versão num novo cliente.

### Fazer o `CodeEditor` consumir todos os resultados

O outline não pertence ao editor individual e a seleção da tab atual é uma
decisão do workspace. Isso colocaria responsabilidades de apresentação global
no documento.

### Criar um router para todos os resultados LSP

Seria uma interface larga e misturaria completion, menus, referências,
diagnósticos e workspace edits. O módulo atual fica deliberadamente profundo
para apenas os resultados com a mesma regra de coerência.

## Consequences

A regra URI + versão tem uma única implementação e um teste de integração
através dos signals reais de `LspClient`. O `MainWindow` perde wiring
duplicado, mas continua a decidir navegação, apresentação e ações de workspace.

## Verification

`testLspEditorResultRouterRejectsStaleDocumentResults` verifica que formatting
e symbols antigos são ignorados e que resultados da versão atual atravessam o
router.
