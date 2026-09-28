# ADR 0048: Isolar a projeção do tema do application shell

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow::applyTheme` aplicava diretamente várias camadas visuais:

- palette global do Qt;
- stylesheet do `QMainWindow`;
- aparência de tabs;
- handles do splitter;
- breadcrumbs;
- labels da status bar.

A função dependia simultaneamente de `ThemeManager`, `ApplicationStyle`,
`AppearanceController` e de vários widgets concretos. Isso fazia o composition
root conhecer detalhes de materialização do tema e tornava alterações visuais
espalhadas pela janela.

`ThemeManager` continua sendo a fonte de tokens semânticos e
`AppearanceController` continua sendo a fonte de preferências de fonte. A
projeção desses valores para as superfícies do shell é uma responsabilidade
distinta.

## Decision

Criar `ApplicationThemeController` com uma interface de uma operação:
`apply()`. O módulo recebe as superfícies do shell no construtor e materializa:

- a palette global;
- o stylesheet global;
- a fonte e as regras específicas de tabs, splitter e breadcrumbs;
- a atualização temática da status bar.

`MainWindow` continua responsável por decidir quando o tema deve ser aplicado,
por exemplo após uma mudança de tema ou de aparência, mas não conhece mais as
regras CSS concretas. `ApplicationThemeController` não persiste preferências,
não escolhe temas e não traduz textos.

## Alternatives considered

### Manter a aplicação do tema em `MainWindow`

Rejeitado porque transforma a janela em dona dos detalhes visuais de todas as
superfícies e torna cada ajuste de design uma alteração no composition root.

### Colocar as regras no `ThemeManager`

Rejeitado porque `ThemeManager` fornece tokens e palette; fazê-lo conhecer
widgets e CSS inverteria a separação entre fonte de estilo e sua projeção.

### Criar um controller por widget

Rejeitado porque dividir tabs, splitter, breadcrumbs e status bar em módulos
independentes perderia a coerência de uma aplicação atômica do tema e criaria
interfaces rasas.

## Consequences

Alterações na linguagem visual do shell ficam localizadas no controller e nos
documentos de design correspondentes. A janela mantém uma interface pequena
para reaplicar o tema, e o comportamento pode ser testado com widgets mínimos
sem construir a aplicação inteira.

Futuras sessões que alterarem cores, espaçamentos, estados de foco ou regras
de tabs/splitter/breadcrumbs devem consultar os pares em `docs/design/` e
atualizar o `.md` e o `.html` do aspecto afetado.

## Verification

- `testApplicationThemeControllerProjectsThemeToShell` verifica a projeção
  para janela, tabs, splitter e breadcrumbs.
- O build CMake, o CTest e `git diff --check` passam após a extração.
