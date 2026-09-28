# ADR 0047: Fazer o LSP runtime controller possuir o event intake das settings

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` ligava manualmente os signals de `SettingsPanel` e
`LspManagerDialog` ao `LspRuntimeController`. A mesma política aparecia em
vários grupos de lambdas:

- enable/disable do runtime;
- refresh e limpeza do cache;
- preferência por runtime online;
- enablement e caminho do clangd.

O `LspRuntimeController` já possuía os dois painéis, o runtime Zith, o cliente
clangd e os métodos que mutam esse estado. Portanto, manter o event intake na
janela fazia o composition root conhecer detalhes das superfícies de
configuração e deixava a interface do controller incompleta.

## Decision

`LspRuntimeController` passa a ligar internamente os signals de configuração
dos dois painéis e a traduzir esses eventos para sua política:

- `lspEnabledChanged` chama `setEnabled`;
- refresh e clear chamam as operações de runtime somente quando aplicável;
- a preferência online é persistida, aplicada ao runtime e, se ativo,
  provoca refresh;
- enablement e caminho do clangd são persistidos e provocam reconciliação.

As diferenças de infraestrutura continuam atravessando callbacks pequenos:
persistência e reconciliação permanecem adapters fornecidos por `MainWindow`.
O controller não passa a conhecer `TomlSettingsStore` nem a janela.

## Alternatives considered

### Manter todos os signals em `MainWindow`

Rejeitado porque duplicava a política entre dois painéis e mantinha detalhes de
configuração no composition root, apesar de o controller já ser o dono das
transições resultantes.

### Criar um `LanguageServiceSettingsController`

Rejeitado porque criaria um segundo módulo apenas para encaminhar eventos ao
`LspRuntimeController`. A seam já existente no runtime controller é mais
profunda: ela combina entrada, enablement, persistência por adapter e
reconciliação.

### Colocar persistência diretamente no `LspRuntimeController`

Rejeitado porque acoplaria a política de runtime ao armazenamento global e
reduziria a possibilidade de testar ou substituir a persistência.

## Consequences

`MainWindow` mantém apenas a wiring de ações de navegação e diálogos
(`Preferences`, `Shortcuts` e `LSP Manager`); deixa de repetir as transições de
runtime e clangd. A interface de `Callbacks` cresce com adapters semânticos de
persistência, mas a política e o ordering dos eventos ficam locais e
testáveis.

## Verification

- `testLspRuntimeControllerOwnsSettingsEventWiring` emite os signals do
  `LspManagerDialog` e verifica persistência e reconciliação.
- O build CMake, o CTest e `git diff --check` passam após a mudança.
