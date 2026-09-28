# ADR 0027: Isolar o diálogo de onboarding

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow.cpp` continha a implementação completa de `GettingStartedDialog`,
incluindo layout, atalhos, dicas, persistência da opção de não mostrar e
estilização dependente do tema. O diálogo não precisava de estado da janela
nem de callbacks do shell.

Manter uma classe de UI concreta dentro do arquivo do shell aumentava o custo
de navegar e alterar a composição principal, além de esconder um módulo
reutilizável atrás de uma implementação local.

## Decision

`GettingStartedDialog` passa a ser um módulo próprio em
`editor/panels/GettingStartedDialog.h/.cpp`. A interface pública é apenas o
construtor com o parent Qt; o módulo encapsula a apresentação, o texto
onboarding e a persistência da preferência de dismiss.

`MainWindow` continua decidindo quando abrir o diálogo, mas não conhece mais
seu layout, seus controles ou sua política de tema.

## Alternatives considered

### Manter a classe local

Evitaria arquivos adicionais, mas manteria uma implementação visual extensa
no shell e impediria testar ou evoluir o onboarding como módulo independente.

### Tornar o onboarding parte de `WelcomeWidget`

Misturaria a tela inicial persistente com um diálogo modal de ajuda, apesar de
terem ciclos de vida e objetivos diferentes.

## Consequences

Alterações no onboarding ficam locais e o `MainWindow` perde uma dependência
de detalhes de layout. O diálogo ainda usa stores e tema globais, uma decisão
conservadora que preserva o comportamento existente; isso pode ser refinado
se surgir um segundo host de onboarding.

## Verification

O alvo `Helios` e a suíte `HeliosTests` continuam compilando após a extração.
