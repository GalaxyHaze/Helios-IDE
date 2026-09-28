# ADR 0063: Extrair o parsing do estado Git

- Status: Accepted
- Date: 2026-09-27

## Context

`GitPanel` executava o processo Git, mantinha a fila de operações, apresentava
feedback, interpretava `git status --short --branch` e construía widgets para
cada ficheiro alterado. O parsing era pequeno, mas estava acoplado à criação
de `QListWidgetItem`, cores e caminhos locais, impedindo testar o contrato do
formato sem criar o painel.

A duplicação entre interpretação e renderização também tornava ambíguo onde
deveriam ficar regras como o tratamento de renames e a distinção entre branch e
entries de ficheiros.

## Decision

Criar `GitStatusParser` no core, com os value objects:

- `GitStatusEntry`, contendo o status de duas colunas e o caminho relativo;
- `GitStatusSnapshot`, contendo branch e entries;
- `GitStatusParser::parse`, que converte o output textual para o snapshot e
  normaliza o alvo de uma linha de rename.

`GitPanel` continua dono de:

- iniciar e terminar processos Git;
- serializar operações concorrentes e refresh pendente;
- decidir mensagens de sucesso/erro;
- resolver caminhos relativos contra o root atual;
- construir e estilizar os rows visuais.

O parser não conhece `QWidget`, `QProcess`, `ThemeManager` ou filesystem.

## Alternatives considered

### Manter o parsing no painel

Rejeitado porque mistura uma regra de formato estável com apresentação e
impede testes pequenos do contrato de Git.

### Criar um controller para cada operação Git

Rejeitado porque dividiria `stage`, `commit` e `refresh` sem remover o
acoplamento mais básico entre parsing e UI. A unidade útil neste momento é o
snapshot produzido pelo comando.

### Fazer o parser resolver caminhos absolutos

Rejeitado porque introduziria filesystem e root mutável num módulo que deve
apenas interpretar output. A resolução continua no painel, que já possui o
workspace ativo.

## Consequences

O parsing passa a ser testável sem widgets e o painel recebe uma estrutura
semântica antes de renderizar. A interface também deixa explícito que o
status de Git é relativo ao output do comando, enquanto a decisão de abrir um
ficheiro continua sendo uma responsabilidade da apresentação.

O parser cobre o formato atualmente solicitado (`--short --branch`); se o
comando mudar para outro formato, essa alteração fica localizada no parser e
no seu teste, não distribuída pela construção dos rows.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `testGitStatusParserExtractsBranchAndPaths`
- `git diff --check`
