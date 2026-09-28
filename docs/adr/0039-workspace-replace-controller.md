# ADR 0039: Isolar a aplicação de replace-all no workspace

- Status: Accepted
- Date: 2026-09-27

## Context

Depois de o `SearchPanel` concluir o scan, `MainWindow` ainda concentrava a
confirmação do utilizador e a aplicação dos targets. A mesma função precisava
tratar tabs abertas, sincronizar alterações pendentes do LSP, escrever ficheiros
fechados de forma segura, reportar falhas e atualizar novamente a pesquisa.

Isso fazia o composition root conhecer detalhes de mutação de texto e
duplicava a distinção entre documento materializado e ficheiro fechado.

## Decision

`WorkspaceReplaceController` passa a possuir a política de replace-all. A
interface recebe apenas o pedido e os targets do scan. Adapters fornecidos
pelo host resolvem:

- o editor aberto correspondente a um path;
- confirmação com contagem e resumo da operação;
- publicação de mensagens de estado;
- refresh da apresentação da pesquisa.

Para um editor aberto, o controller aplica edits no documento, marca-o como
modificado e envia as alterações pendentes ao LSP. Para um ficheiro fechado,
lê o conteúdo, calcula os edits e grava via `QSaveFile`. A contagem final usa
os edits efetivamente aplicados, não apenas a previsão do scan.

## Alternatives considered

### Manter a operação em `MainWindow`

Evitaria uma nova seam, mas deixaria uma política de mutação com múltiplos
paths de erro no composition root e tornaria difícil testá-la sem construir a
janela inteira.

### Fazer `SearchPanel` aplicar as alterações

Misturaria scan/apresentação com mutação de documentos e ficheiros, além de
fazer o painel conhecer tabs, LSP e status bar.

### Criar adapters separados para tabs e ficheiros sem controller

Distribuiria a confirmação, contagem e refresh entre vários pontos, sem uma
única invariável para a operação completa.

## Consequences

Replace-all tem uma interface pequena e uma política local, testável com
adapters em memória e ficheiros temporários. `SearchPanel` continua dono do
scan e do preview; `MainWindow` continua dono da composição e dos diálogos,
mas já não possui a mutação. O controller continua dependente de Qt para
`CodeEditor` e `QSaveFile`, porque essa é a seam concreta do workspace atual.

## Verification

`testWorkspaceReplaceControllerUpdatesOpenAndClosedTargets` verifica
confirmação, aplicação nos dois tipos de target, contagem efetiva, refresh e
estado modificado do editor aberto.
