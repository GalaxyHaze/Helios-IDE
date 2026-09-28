# ADR 0080: Extrair a sessão de operações do repositório Git

- Status: Accepted
- Date: 2026-09-27

`GitPanel` ainda coordenava a política inteira de source control depois da
extração do parser e do executor: construía argumentos, mantinha a operação
ativa, enfileirava refresh, encadeava `status` com `remote` e traduzia falhas
em mensagens. A decisão é introduzir `GitRepositorySession`, que concentra
esse workflow atrás de intenções de repositório e publica snapshots,
disponibilidade, busy state e mensagens; o painel permanece responsável por
apresentação e interação.

## Considered Options

- **Manter a política no painel**: rejeitado porque cada nova operação
  aumentaria o acoplamento entre estado assíncrono e widgets.
- **Criar um controller separado por operação**: rejeitado porque dividiria um
  workflow que precisa de uma única serialização e de refresh pós-operação,
  produzindo seams rasas e estado duplicado.
- **Criar uma sessão de repositório**: escolhido porque dá localidade ao
  workflow completo e oferece uma interface pequena para a UI e para testes.

## Consequences

- `GitPanel` não conhece `GitCommandResult`, argumentos Git, timeout ou fila de
  refresh.
- A sessão é o único lugar que define a ordem `status -> remote` e a
  atualização depois de stage, unstage, commit, init ou remote.
- Mensagens são publicadas como resultado de operação; o painel decide apenas
  como as apresenta.
- A sessão continua usando `GitCommandRunner` concreto; não foi criada uma
  abstração genérica de shell sem uma segunda implementação real.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `GitRepositorySession` com executável Git falso temporário
- `git diff --check`
