# ADR 0067: Extrair a instalação do runtime do Zith

- Status: Accepted
- Date: 2026-09-27

`ZithToolchainManager` também misturava a coordenação de downloads com a
materialização dos artefactos no cache: cópia do executável, permissões,
extração de `tar`/PowerShell e limpeza parcial quando a extração falhava. A
decisão é colocar essa política em `ZithRuntimeInstaller`, que recebe um
`ZithRuntimeInstallRequest` e retorna sucesso ou uma mensagem de erro. O
manager continua dono da ordem assíncrona, fallback e decisão de quando
instalar; `ZithRuntimeCatalog` continua dono dos paths e da validação de
instalações completas.

Esta seam concentra comportamento dependente do sistema operativo atrás de
uma operação pequena e permite testar a instalação sem rede. Não foi criada
uma abstração para `QProcess`: o installer tem um único mecanismo de execução
no repositório e uma segunda implementação ainda não existe para justificar
esse seam.

## Consequences

- O lifecycle de rede não conhece permissões nem comandos de extração.
- Falhas de instalação e limpeza ficam localizadas no mesmo módulo.
- A instalação pode ser exercitada com um ficheiro temporário determinístico.
- O manager ainda contém a política de download e fallback, deliberadamente
  fora do escopo desta extração.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no installer
- `git diff --check`
