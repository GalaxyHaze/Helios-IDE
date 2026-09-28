# ADR 0126: Extrair o codec de persistência TOML das settings

- Status: Accepted
- Date: 2026-09-28

`TomlSettingsStore` acumulava duas responsabilidades diferentes: manter o
estado vivo das preferências da aplicação e interpretar/serializar o formato
TOML. Essa combinação fazia com que alterações no formato tocassem no módulo
que também possui auto-save, aliases de compatibilidade e notificações para o
editor.

A decisão é introduzir `TomlSettingsSnapshot` e `TomlSettingsCodec`. O snapshot
é o valor de transferência completo entre estado vivo e persistência. O codec
é puro: lê e escreve TOML, aplica defaults, normaliza limites e preserva a
migração das chaves antigas `fontFamily`/`fontSize`. `TomlSettingsStore`
continua sendo a composition seam da configuração: possui o caminho do
ficheiro, o estado materializado, `load`/`save`, setters e sinais.

## Alternativas

- Manter parsing e serialização no store: rejeitado porque o módulo continuaria
  a mudar por razões de formato e por razões de lifecycle.
- Criar um objeto de configuração global adicional: rejeitado porque duplicaria
  o estado vivo e enfraqueceria o ownership do store.
- Usar um parser TOML externo nesta etapa: rejeitado porque o formato atual é
  deliberadamente pequeno e a extração deve reduzir a superfície sem introduzir
  uma dependência de runtime.

## Consequências

- O contrato do formato pode ser testado sem instanciar `QObject` ou escrever
  ficheiros.
- A compatibilidade legada e a normalização dos valores ficam concentradas numa
  seam única.
- O store permanece responsável por efeitos observáveis: I/O, auto-save e
  `editorPreferencesChanged`.
- O codec atual continua a ser um parser TOML restrito ao schema do Helios; não
  pretende implementar a linguagem TOML completa.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- testes diretos de round-trip, migração e normalização do
  `TomlSettingsCodec`
- `git diff --check`
- `cppcheck` e `clang-tidy` no codec e no store; os avisos restantes são os
  falsos positivos Qt/MOC já conhecidos e sugestões de estilo genéricas.
