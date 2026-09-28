# ADR 0140: Seam de persistência do layout da janela

- Status: Accepted
- Date: 2026-09-28

`MainWindow` conhecia diretamente os detalhes de persistência da geometria,
do estado de docks, da largura da sidebar e da visibilidade dos painéis. A
mesma política aparecia em callbacks passados a `SidebarController` e
`WorkspacePanelPresentationController`, enquanto a restauração ocorria em
ordem diferente do salvamento. Isso fazia o composition root conhecer tanto a
composição visual como o storage TOML e dificultava testar a política sem
construir a janela completa.

A decisão é criar `WindowLayoutPersistence` como uma interface pequena e
`TomlWindowLayoutPersistence` como o adapter do `TomlSettingsStore`.
`WindowLayoutController` concentra a política de restauração e salvamento:
aplica geometry/state no startup, aplica largura e visibilidade sem regravar o
storage durante a restauração, e persiste apenas mutações de splitter ou de
visibilidade. `MainWindow` continua sendo o composition root e mantém o
ownership dos widgets.

## Alternativas

- Manter chamadas diretas em `MainWindow`: rejeitado porque espalha a política
  de layout por callbacks e mistura Qt com o formato de persistência.
- Criar um wrapper genérico para todo o `TomlSettingsStore`: rejeitado porque
  seria uma interface rasa e ampliaria o seam para preferências sem relação
  com layout.
- Fazer `WindowLayoutController` possuir ou criar o settings store: rejeitado
  porque esconderia ownership global e impediria um fake de persistência nos
  testes.
- Persistir o layout apenas no encerramento: rejeitado porque largura e
  visibilidade são preferências efetivas e a aplicação já espera que mudanças
  sobrevivam a alterações individuais e encerramentos anormais.

## Consequências

- A política de layout tem um seam pequeno, profundo e testável com fake.
- O adapter TOML é o único módulo que traduz o contrato de layout para o
  `TomlSettingsStore`.
- A restauração deixa de causar escritas redundantes no startup.
- `SidebarController` e `WorkspacePanelPresentationController` recebem uma
  política de layout do host sem conhecer o storage.
- `MainWindow` ainda compõe a janela, porque ownership de widgets e ordem de
  inicialização continuam sendo responsabilidades do shell.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
