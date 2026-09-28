# ADR 0132: Remover paths de runtime específicos do host

- Status: Accepted
- Date: 2026-09-28

`ZithToolchainManager` continha paths absolutos para checkouts locais de
desenvolvimento em `/home/diogo`. Essa política contradizia a resolução
dinâmica documentada, tornava o comportamento dependente da máquina do autor e
podia selecionar um executável fora do cache sem qualquer configuração
explícita.

A decisão é remover a descoberta implícita de checkouts locais. O runtime
gerido continua a ser resolvido pelo catálogo e os ambientes de
desenvolvimento usam o par explícito
`HELIOS_ZITH_LSP_PATH`/`HELIOS_ZITH_STDLIB_PATH`. O manager continua a remover
o cache legado com a tag `local`, para que versões antigas não contaminem a
resolução atual.

## Alternativas

- Derivar paths a partir de `HOME`: rejeitado porque ainda seria uma
  convenção não configurada e não portável entre layouts.
- Manter os paths e apenas parametrizá-los em CMake: rejeitado porque faria a
  build carregar uma política de ambiente de desenvolvimento.
- Criar um detector de checkout local configurável: adiado até existir uma
  necessidade de produto distinta dos overrides explícitos.

## Consequências

- Builds em outras máquinas e plataformas deixam de depender do filesystem do
  autor.
- O fluxo local continua disponível, mas torna a intenção explícita através
  dos overrides documentados.
- A resolução normal fica centrada no catálogo gerido, no cache e na release
  remota.
- Caches antigos com tag `local` continuam a ser limpos durante a migração.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
