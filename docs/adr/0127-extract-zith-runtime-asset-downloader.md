# ADR 0127: Extrair o downloader de assets do runtime Zith

- Status: Accepted
- Date: 2026-09-28

`ZithToolchainManager` coordenava a política de resolução do runtime, mas
também conhecia os detalhes de cada download: criação do cliente de rede,
timeout, abortamento de `QNetworkReply`, escrita do payload e limpeza de
ficheiros temporários. Essas responsabilidades mudavam por razões diferentes e
tornavam o manager uma seam pouco local para alterações de transporte.

A decisão é introduzir `ZithRuntimeAssetDownloader`. O módulo recebe um pedido
de um asset e materializa-o num caminho temporário, expondo apenas conclusão ou
falha. Ele possui validação do pedido, timeout, cancelamento e gravação
atómica. `ZithToolchainManager` continua responsável por escolher a fonte do
runtime, manter a fila semântica de assets, instalar cada asset e decidir
fallback ou readiness.

## Alternativas

- Manter toda a rede no manager: rejeitado porque mistura infraestrutura de
  transporte com política de resolução e torna cada mudança de I/O mais ampla.
- Extrair apenas funções auxiliares de escrita: rejeitado porque deixaria
  `QNetworkReply`, timeout e cancelamento acoplados ao manager.
- Fazer o downloader instalar diretamente o runtime: rejeitado porque
  confundiria transporte com a política de instalação e impediria o manager de
  escolher fallback entre releases.

## Consequências

- O manager tem uma interface de transporte pequena e conserva ownership das
  decisões de runtime.
- A gravação usa `QSaveFile`, evitando deixar um asset parcialmente escrito no
  cache após falha.
- O downloader pode ser testado sem construir o manager; requests incompletos
  falham de forma síncrona e explícita.
- A consulta da metadata da release mais recente permanece no manager por
  enquanto, porque tem headers e semântica diferentes dos downloads de assets.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de validação de request do
  `ZithRuntimeAssetDownloader`
- `git diff --check`
- `cppcheck` e `clang-tidy` no novo downloader; avisos restantes são os
  falsos positivos Qt/MOC e sugestões de estilo genéricas.
