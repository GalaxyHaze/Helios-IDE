# ADR 0019: Routing coerente de referências LSP

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` recebia `referencesResult` de mais de um cliente LSP e
preenchia o painel de referências sem verificar a URI ou a versão do
documento que originou a solicitação. Uma resposta atrasada podia, portanto,
substituir o resultado da tab ativa ou reaparecer depois de o utilizador
mudar de documento.

## Decision

`LspReferencesRouter` recebe as tabs, o painel de referências e uma pequena
função de apresentação. Para cada cliente LSP associado, ele só aceita um
resultado quando:

- a tab ativa é um `CodeEditor`;
- a URI recebida corresponde à URI do editor ativo;
- a versão recebida corresponde à versão atual do editor.

Resultados coerentes são renderizados no painel. O painel só é aberto quando
há pelo menos uma referência; a decisão de como mostrá-lo continua no shell,
através da função injetada.

## Alternatives considered

### Manter o lambda em `MainWindow`

Seria pouco código, mas deixaria a janela responsável por uma invariável
assíncrona e repetível para cada cliente LSP. Também dificultaria testar
respostas atrasadas sem construir a janela completa.

### Aceitar qualquer resposta da URI

Isso continuaria permitindo que uma resposta de uma versão antiga sobrescreva
o estado atual do painel. A versão é parte da identidade temporal do
documento, não apenas um detalhe do transporte.

### Fazer o painel decidir se o resultado é atual

O painel não conhece tabs, documentos ativos ou versões LSP. Colocar essa
política nele misturaria apresentação com routing e reduziria a profundidade
das duas interfaces.

## Consequences

Resultados atrasados ou pertencentes a outra tab são descartados em um único
seam. `MainWindow` continua responsável apenas por compor clientes, painel e
visibilidade. O router depende de `QTabWidget` e `ReferencesPanel` porque a
coerência do resultado é definida pelo editor ativo.

## Verification

`testLspReferencesRouterRejectsStaleAndInactiveResults` verifica descarte por
versão, descarte por tab ativa e aceitação de uma resposta coerente.
