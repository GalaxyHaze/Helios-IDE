# ADR 0066: Extrair o catálogo de releases do Zith

- Status: Accepted
- Date: 2026-09-27

`ZithToolchainManager` misturava a orquestração assíncrona de rede com o
formato JSON da API de releases do GitHub e com a seleção de artefactos por
plataforma. A decisão é colocar a interpretação do payload e a seleção do
LSP/stdlib compatíveis em `ZithReleaseCatalog`, uma interface estática e sem
I/O; o manager continua dono de requests, fallback, downloads e instalação,
enquanto `ZithRuntimeCatalog` continua dono do cache instalado.

Esta seam foi escolhida porque concentra uma política estável e testável sem
rede: tags inválidas e assets incompletos são rejeitados num único ponto, e a
seleção de nomes específicos da plataforma não precisa ser repetida no
lifecycle. Não foi criada uma abstração de transporte, porque o repositório
tem apenas um adapter de rede e essa variação ainda não justifica uma
interface adicional.

## Consequences

- O parser pode ser verificado com payloads determinísticos, sem `QNetworkReply`.
- O manager passa a coordenar resultados de release em vez de conhecer o JSON do GitHub.
- A matriz de nomes de artefactos continua dependente da plataforma, mas está
  localizada num único módulo.
- Instalação, cache e fallback permanecem no manager nesta tranche; uma futura
  extração só deve ocorrer se existir uma seam com interface mais profunda que
  um simples encaminhamento.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` em `ZithReleaseCatalog.cpp` e `ZithToolchainManager.cpp`
- `git diff --check`
