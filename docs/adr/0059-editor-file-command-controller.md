# ADR 0059: Isolar a política de comandos de ficheiro do shell

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` tratava diretamente os diálogos de abrir e guardar, criava tabs
untitled, escolhia o caminho de Save As, traduzia `SaveResult` para mensagens
de status e era chamado tanto pelo shell como pelo Vim e pelo fecho de tabs.
Isso fazia a janela conhecer a mesma política de ficheiros através de vários
callbacks.

O `EditorSessionController` já possuía a invariável de escrita, rebind de
untitled para ficheiro nomeado e sincronização LSP, mas não deveria conhecer
diálogos nem a apresentação do shell.

## Decision

Criar `EditorFileController` como módulo de entrada para comandos de ficheiro.
Ele possui:

- criação de ficheiros untitled;
- seleção de caminhos para abrir e Save As;
- encaminhamento de abrir para `EditorSessionController`;
- tradução dos resultados de gravação para status;
- atualização da disponibilidade de comandos após uma gravação bem-sucedida;
- tratamento comum de `NewFile`, `OpenFile` e `SaveFile`.

`EditorSessionController` continua dono da escrita, do estado modificado e da
sincronização de documentos. `MainWindow` fornece apenas o parent dos diálogos,
o root inicial, o feedback e o refresh de disponibilidade. O controller de
ficheiros também é usado pelo Vim e pelo `EditorTabCloseController`, evitando
que esses caminhos voltem a duplicar Save As.

## Alternatives considered

### Manter os diálogos em `MainWindow`

Rejeitado porque shell, Vim e fecho de tabs continuariam a entrar por fluxos
distintos e a política de Save As permaneceria no composition root.

### Colocar diálogos em `EditorSessionController`

Rejeitado porque misturaria UI modal com escrita, document-sync e restauração de
sessão, reduzindo a reutilização e a testabilidade do módulo de sessão.

### Criar um controller separado para cada comando

Rejeitado porque New, Open e Save partilham o mesmo editor session e a mesma
política de root/feedback; a divisão perderia profundidade e criaria adapters
rasos.

## Consequences

Os pontos de entrada de comandos de ficheiro partilham uma única política. A
janela deixa de conter os detalhes de Save As e fica menor como composition
root. O módulo ainda depende de widgets Qt para os diálogos, mas essa
dependência está confinada à interação com o utilizador, não ao domínio da
sessão.

## Verification

`testEditorFileControllerOwnsFileCommandPolicy` verifica Save, feedback,
refresh de disponibilidade e New File através da interface do controller. O
fluxo de abertura continua coberto pelas integrações de navegação existentes.
