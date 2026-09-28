# ADR 0136: Dependências nomeadas para comandos do workspace

- Status: Accepted
- Date: 2026-09-28

`WorkspaceCommandController` recebia um `LspClient` e um `CompilerPanel` como
argumentos posicionais, além de callbacks nomeados. Esses dois colaboradores
participam tanto do fluxo principal de comandos como da criação do
`WorkspaceTaskOutputController`, por isso representam a infraestrutura estável
da implementação, não políticas independentes do host.

A decisão é agrupá-los em
`WorkspaceCommandController::Dependencies`. Os callbacks continuam no objeto
`Callbacks`, pois expressam políticas e efeitos fornecidos pelo composition
root: enablement, resolução do workspace, seleção do editor, persistência e
feedback. O controller continua sem ownership do cliente ou do painel.

## Alternativas

- Manter os ponteiros posicionais: rejeitado porque o wiring misturava
  colaboradores persistentes com uma lista já extensa de políticas.
- Agrupar ponteiros e callbacks num único contexto: rejeitado porque apagaria a
  distinção entre infraestrutura colaboradora e política do host.
- Fazer o controller criar o `LspClient` ou o painel: rejeitado porque
  transferiria ownership da janela para um módulo de comandos.

## Consequências

- A composição deixa explícito quais objetos o controller usa como superfície
  de execução e saída.
- Os testes montam o seam sem depender da ordem dos ponteiros.
- O tracker de tarefas continua com os mesmos colaboradores e lifecycle.
- A interface passa a ter uma estrutura adicional, mas sem criar um contexto
  global ou um novo nível de indireção.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
