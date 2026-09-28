# ADR 0072: Extrair a execução assíncrona de comandos Git

- Status: Accepted
- Date: 2026-09-27

`GitPanel` misturava apresentação do estado do repositório com o lifecycle de
`QProcess`: configuração do diretório de trabalho, captura de stdout/stderr,
timeout, kill, falha de arranque e classificação do exit status. Essa mistura
tornava cada operação Git dependente dos detalhes do processo.

A decisão é introduzir `GitCommandRunner`. O módulo recebe um programa, um
diretório de trabalho e argumentos, e emite um `GitCommandResult` tipado. O
resultado distingue sucesso, falha ao iniciar e timeout. O runner mantém um
único comando em voo e rejeita uma nova execução enquanto o processo anterior
está ativo.

`GitPanel` continua dono da política de produto: status, remote, stage,
unstage, commit, inicialização, conexão ao GitHub, mensagens e refresh
pendente. Ele apenas associa cada resultado à operação que iniciou.

## Consequences

- O lifecycle de processos Git fica concentrado em uma seam testável.
- O painel deixa de depender de `QProcess` e `QTimer`.
- Falhas de arranque e timeout têm uma representação explícita, sem inferência
  a partir de sinais de baixo nível.
- A execução continua concreta e local; não foi criado um adapter genérico de
  shell sem uma segunda implementação necessária.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `cppcheck` e `clang-tidy` no runner
- `git diff --check`
