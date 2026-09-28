# ADR 0049: Isolar a projeção de traduções do shell

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow::applyTranslations` conhecia simultaneamente as chaves de
tradução dos menus e os atalhos textuais da `ActivityBar`. A função também era
chamada pelo ciclo de locale, fazendo a janela acumular duas responsabilidades:
carregar/selecionar o locale e materializar a linguagem do shell.

`ShellCommandSurface` já possui a sua própria projeção de textos de menus, mas
o composition root ainda precisava coordená-la junto com os tooltips da
activity bar. Alterar uma chave ou adicionar um modo exigia editar a janela.

## Decision

Criar `ShellTranslationController` com uma operação `apply()`. O módulo:

- chama a projeção de traduções do `ShellCommandSurface`;
- aplica os textos traduzidos e atalhos dos quatro modos da `ActivityBar`;
- centraliza as chaves e o formato dos tooltips do shell.

`MainWindow` continua responsável por detectar a mudança de locale, pedir ao
`TranslationManager` para carregá-lo e atualizar o estado dos settings. O
controller não escolhe locales, não persiste preferências e não traduz os
conteúdos internos dos painéis.

## Alternatives considered

### Manter os tooltips em `MainWindow`

Rejeitado porque deixa conhecimento de chaves e formato de atalhos no
composition root, além de duplicar a coordenação visual dos menus.

### Fazer `ActivityBar` observar diretamente o `TranslationManager`

Rejeitado porque acoplaria um widget de navegação à fonte global de tradução e
espalharia a política de atualização entre widgets individuais.

### Fundir tradução e tema num único controller

Rejeitado porque tema e locale têm fontes, ciclos de mudança e invariantes
distintos. Um controller único teria interface mais larga e menor localidade.

## Consequences

As mudanças de texto do shell ficam localizadas no controller e no
`ShellCommandSurface`; `MainWindow` conserva apenas a política de carregamento
do locale. O comportamento é testável com uma activity bar e uma menu bar
sem construir a janela completa.

Alterações de linguagem visual ou textual que também mudem tooltips, menus,
atalhos apresentados ou hierarquia do shell devem consultar os documentos
correspondentes em `docs/design/` e atualizar o par `.md` + `.html` quando o
aspecto gráfico for afetado.

## Verification

- `testShellTranslationControllerProjectsShellText` verifica a projeção para
  os menus e os quatro botões da activity bar.
- O build CMake, o CTest e `git diff --check` passam após a extração.
