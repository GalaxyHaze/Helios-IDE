# ADR 0037: Isolar a apresentação dos painéis opcionais do workspace

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` tratava diretamente a visibilidade do Outline e do Bottom Panel.
Além de mostrar ou esconder widgets, cada transição tinha efeitos associados:
persistir `outlineVisible`, limpar símbolos ao esconder o Outline, atualizar o
editor ativo ao reabri-lo, sincronizar os itens checkable do menu e converter
o fecho interno do Bottom Panel numa transição de shell.

Essas regras estavam divididas entre o switch de `ShellCommand`, uma função
auxiliar da janela e um lambda ligado a `BottomPanel::closeRequested`.

## Decision

`WorkspacePanelPresentationController` possui a política de apresentação dos
dois painéis opcionais. A sua interface oferece operações semânticas para
mostrar/esconder ou alternar cada painel e uma sincronização explícita com o
shell.

O controlador:

- persiste a visibilidade do Outline;
- limpa símbolos quando o Outline é escondido;
- pede atualização do editor ativo quando o Outline é mostrado;
- transforma o fecho do Bottom Panel numa transição coerente;
- mantém os itens checkable de `ShellCommandSurface` sincronizados.

`OutlinePanel` e `BottomPanel` continuam donos do conteúdo e das interações
internas. `MainWindow` continua a compor os widgets e fornece callbacks, mas
não repete a política de transição.

## Alternatives considered

### Manter a política em `MainWindow`

Seria menor no curto prazo, mas exigiria que cada nova origem de visibilidade
conhecesse persistência, limpeza, refresh e sincronização de menu.

### Criar um controlador por painel

Duplicaria a mesma regra de publicação no shell e não representaria a
invariante compartilhada: a visibilidade de um painel deve ser coerente com o
comando checkable correspondente.

### Fazer `BottomPanel` ou `OutlinePanel` possuir o shell

Misturaria conteúdo de painel com composição da janela e faria um painel
conhecer menus, settings e o editor ativo.

## Consequences

A apresentação opcional do workspace tem uma única política e uma superfície
de teste pequena. O controlador conhece as superfícies Qt envolvidas, mas não
conhece LSP, contexto ou conteúdo de diagnostics. Novos painéis opcionais
devem demonstrar uma regra compartilhada antes de serem adicionados a este
módulo.

## Verification

`testWorkspacePanelPresentationKeepsPanelAndShellStateCoherent` verifica
persistência e refresh do Outline, sincronização dos comandos e o caminho de
fecho do Bottom Panel.
