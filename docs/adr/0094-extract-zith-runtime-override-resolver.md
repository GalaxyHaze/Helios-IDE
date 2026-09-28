# ADR 0094: Extrair a resolução de overrides do runtime Zith

- Status: Accepted
- Date: 2026-09-27

`ZithToolchainManager` combinava leitura dos overrides de ambiente,
validação dos paths, classificação de configurações parciais e emissão de
status com a resolução, instalação e fallback do runtime gerido. Isso fazia
com que uma política determinística dependesse do ciclo de vida do manager.

A decisão é criar `ZithRuntimeOverrideResolver` como um módulo puro. A sua
interface recebe os dois paths e devolve uma ação (`NotConfigured`,
`IgnoreAndContinue`, `Fail` ou `Use`), uma identidade de runtime quando
aplicável e uma mensagem. `ZithToolchainManager` continua dono dos sinais,
fallback, cache e transições de lifecycle.

## Considered Options

- **Manter a classificação no `ZithToolchainManager`**: rejeitado porque
  mistura política de configuração com rede, cache, instalação e lifecycle.
- **Fazer o resolver ler diretamente o ambiente**: rejeitado porque esconderia
  uma dependência global e reduziria a testabilidade da seam.
- **Extrair um resolver puro com paths explícitos**: escolhido porque mantém
  a interface pequena, permite testar todos os estados sem `QObject` e
  preserva a localidade das regras de override.

## Consequences

- A política de overrides pode ser testada sem rede, processos ou widgets.
- `ZithRuntimeCatalog` continua sendo a única interface para layout e
  resolução do cache gerido.
- Overrides parciais continuam permitindo fallback para o runtime local ou
  gerido; paths inválidos quando ambos foram configurados continuam sendo
  falhas terminais.
- A emissão de status e as decisões de fallback permanecem no
  `ZithToolchainManager`, evitando que o módulo puro conheça apresentação ou
  lifecycle.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
