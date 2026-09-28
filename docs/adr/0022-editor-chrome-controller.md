# ADR 0022: Isolar a política de editor chrome

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow::updateEditorChrome` combinava várias responsabilidades:
associar o editor à busca, atualizar breadcrumbs e posição, escolher o rótulo
de linguagem, alterar o título da janela e controlar o debounce de símbolos
do outline. O shell também mantinha quatro campos para comparar URI e versão
de requests assíncronos.

## Decision

`EditorChromeController` expõe `update(CodeEditor *)` e possui internamente:

- atualização do find/replace bar, breadcrumbs, posição, linguagem e título;
- estado de URI e versão do outline;
- debounce do request de símbolos;
- descarte de requests quando o editor ativo já mudou de URI ou versão.

O módulo recebe callbacks para classificar a linguagem, obter o editor atual,
pedir símbolos ao cliente adequado e definir o título. A seleção de
`LspClient` continua no `MainWindow`, porque ela depende do workspace e da
política de linguagem.

## Alternatives considered

### Manter a lógica no `MainWindow`

Evitaria uma classe, mas manteria apresentação, estado assíncrono e seleção
de cliente no mesmo shell, aumentando o custo de qualquer alteração no
outline ou no chrome.

### Colocar a seleção de LSP no controller

Isso faria o chrome conhecer linguagens e runtimes. O controller só define
quando pedir símbolos; o shell continua responsável por escolher o adapter
LSP.

### Dividir cada label em um módulo

Criaria vários módulos rasos com pouca profundidade. O chrome é uma política
coesa: todos os elementos refletem o mesmo editor ativo e o mesmo ciclo de
outline.

## Consequences

O shell perdeu o estado de debounce e uma implementação extensa de
apresentação. O módulo pode ser testado com widgets simples e callbacks
controlados, enquanto o comportamento de outline permanece dependente do
editor ativo.

## Verification

`testEditorChromeControllerUpdatesAndClearsEditorChrome` verifica atualização
e limpeza do chrome através da interface do controller.
