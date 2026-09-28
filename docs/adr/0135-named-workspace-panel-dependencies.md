# ADR 0135: Dependências nomeadas para a apresentação dos painéis do workspace

- Status: Accepted
- Date: 2026-09-28

`WorkspacePanelPresentationController` recebia quatro widgets em posições
fixas, além de duas políticas. Embora os tipos dos widgets fossem distintos,
a lista não comunicava o papel de cada colaborador na apresentação do
workspace e tornava o wiring menos auditável quando a shell mudava.

A decisão é agrupar os colaboradores persistentes em
`WorkspacePanelPresentationController::Dependencies`. Os callbacks continuam
separados porque representam políticas fornecidas pelo composition root:
resolver o editor atual e persistir a preferência de visibilidade. O controller
continua a controlar visibilidade, sincronização dos comandos e atualização do
outline, sem assumir ownership dos widgets.

## Alternativas

- Manter os argumentos posicionais: rejeitado porque a composição escondia a
  relação entre cada widget e a responsabilidade do controller.
- Agrupar widgets e callbacks num contexto geral: rejeitado porque misturaria
  colaboradores da apresentação com políticas do host e ampliaria o seam.
- Extrair um controller por painel: rejeitado porque as operações formam uma
  política coesa de apresentação e sincronização.

## Consequências

- O composition root declara o wiring campo a campo.
- Testes conseguem montar explicitamente a superfície visual.
- Ownership e comportamento permanecem inalterados.
- A interface ganha uma pequena estrutura nomeada, mas não cria um contexto
  global para a janela.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
