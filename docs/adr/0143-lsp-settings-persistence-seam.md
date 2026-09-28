# ADR 0143: Seam de persistência das preferências LSP

- Status: Accepted
- Date: 2026-09-28

`LspRuntimeController` recebia quatro callbacks de persistência para
enablement global, preferência online do runtime Zith, enablement de clangd e
override do caminho de clangd. `MainWindow` também lia os mesmos quatro
valores para configurar routing, runtime e o LSP Manager. A forma concreta do
`TomlSettingsStore` estava, portanto, repetida no composition root e a
interface do controller confundia persistência com efeitos de lifecycle.

A decisão é introduzir `LspSettingsPersistence` como uma interface coesa e
`TomlLspSettingsPersistence` como o adapter TOML. A interface entra em
`LspRuntimeController::Dependencies`; os callbacks do controller ficam
reservados para reconciliação, atualização de ações, confirmação e feedback da
shell. `MainWindow` usa o mesmo adapter para resolver a configuração de
workspace e inicializar o diálogo.

## Alternativas

- Manter quatro callbacks de persistência: rejeitado porque multiplicava uma
  capacidade única e permitia wiring posicional/semântico incoerente.
- Fazer `LspRuntimeController` depender diretamente de
  `TomlSettingsStore::instance()`: rejeitado porque esconderia o storage
  global, reduziria testabilidade e misturaria runtime com formato de arquivo.
- Criar uma interface para todos os settings da aplicação: rejeitado porque
  seria um contexto raso e arrastaria appearance, search e layout para o seam
  de language services.

## Consequências

- A política de runtime não conhece TOML nem singleton.
- O composition root tem uma única fonte adaptada para leituras e escritas de
  preferências LSP.
- Testes exercitam persistência com um fake pequeno e observável.
- A interface preserva a distinção entre estado persistido e efeitos de
  lifecycle/presentation.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
