# ADR 0028: Centralizar a folha de estilo da aplicação

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow.cpp` continha a folha de estilo global da aplicação: menus,
status bar, docks, scrollbars, botões, inputs e estados de foco. Esse bloco
traduz o vocabulário semântico do tema em regras visuais, mas não depende da
composição ou do estado da janela.

Manter essa política no shell misturava decisões de design com wiring de
runtime, documentos e comandos.

## Decision

`ApplicationStyle` expõe `globalStyleSheet()`, uma interface pequena que
materializa a linguagem visual global a partir do `ThemeManager`. `MainWindow`
apenas aplica o resultado durante mudanças de tema/aparência.

O módulo é a implementação compartilhada da política descrita em
`docs/design/visual-language.md`; os previews HTML continuam sendo referências
de design, não uma segunda fonte executável de estilo.

## Alternatives considered

### Manter o stylesheet no `MainWindow`

Seria local no sentido físico, mas manteria decisões gráficas junto de
lifecycle e composição, aumentando o custo de qualquer ajuste visual global.

### Espalhar regras pelos widgets

Daria autonomia local, mas quebraria a coerência do vocabulário semântico e
duplicaria estados de foco, hover e contraste.

### Criar um objeto por grupo de widgets

Seria uma decomposição de baixa profundidade: muitos módulos pequenos para uma
única política de tradução tema-para-style.

## Consequences

O design visual global tem uma seam única e uma superfície de teste pequena.
Alterações de tema continuam sendo aplicadas pelo shell, mas a composição da
folha e seus tokens ficam localizadas. O módulo não conhece documentos,
runtime ou ações.

## Verification

`testApplicationStyleContainsShellAndInputRules` verifica que as famílias
essenciais de regras continuam presentes. O build e a suíte de testes passam
após a extração.
