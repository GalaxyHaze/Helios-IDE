# ADR 0097: Consolidar a interface de capabilities LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` expunha um getter público para cada provider negociado:
completion, hover, definition, execute command e outros. Essa superfície
duplicava o value object `LspServerCapabilities` e fazia cada caller conhecer
os nomes concretos dos campos de protocolo.

A decisão é substituir os getters por `LspClient::supports(Capability)`. O
enum representa o vocabulário semântico consumido pelos módulos da aplicação;
o cliente mantém em um único ponto o mapeamento entre esse vocabulário e
`LspServerCapabilities`.

## Considered Options

- **Manter um getter por provider**: rejeitado porque aumenta a interface
  pública a cada capability nova e espalha a representação do protocolo.
- **Expor diretamente `LspServerCapabilities`**: rejeitado porque callers
  passariam a depender da estrutura protocolar e de seus nomes de campos.
- **Usar strings para consultar capabilities**: rejeitado porque perde
  segurança de tipo e torna erros de spelling runtime failures.
- **Usar um enum e uma operação `supports`**: escolhido porque reduz a
  superfície, centraliza a tradução e preserva segurança de tipo.

## Consequences

- `LspClient` tem uma interface menor para capabilities negociadas.
- Controllers e routers expressam precondições sem depender de getters
  individuais.
- Adicionar uma capability exige atualizar o enum e um único switch no
  cliente, além do value object protocolar quando necessário.
- `documentSyncKind()` continua separado porque representa uma configuração
  numérica de sincronização, não uma capability booleana.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
