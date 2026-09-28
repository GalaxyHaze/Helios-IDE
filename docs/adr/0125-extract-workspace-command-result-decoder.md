# ADR 0125: Extrair o decoder de resultados de comandos do workspace

- Status: Accepted
- Date: 2026-09-28

`WorkspaceCommandController::handleCommandResult` interpretava diretamente
payloads JSON do language service enquanto atualizava o `CompilerPanel`,
publicava diagnósticos e iniciava o tracking de tarefas. Isso fazia com que
as regras de `success`, `programUri`, `taskId` e `codegenAvailable` ficassem
acopladas à apresentação.

A decisão é introduzir `WorkspaceCommandResultDecoder`, um módulo puro que
normaliza o resultado de transporte e o payload JSON para
`WorkspaceCommandResult`. O controller continua responsável por decidir como
apresentar falhas, atualizar o painel, publicar diagnósticos e acompanhar uma
tarefa de execução.

## Alternativas

- Manter o parsing no controller: rejeitado porque cada alteração no protocolo
  exigiria editar código com efeitos visuais e lifecycle.
- Fazer o decoder apresentar mensagens: rejeitado porque misturaria protocolo
  com a linguagem visual do `CompilerPanel`.
- Criar um decoder específico para cada comando (`build`, `check`, `run`):
  rejeitado porque os campos comuns pertencem ao mesmo contrato de resultado;
  a diferença entre comandos é política do controller.

## Consequências

- A semântica de sucesso efetivo é testável sem widgets ou processos LSP.
- Respostas textuais e respostas objeto têm uma representação comum antes da
  apresentação.
- O controller conserva ownership de UI, diagnósticos e task tracking.
- Campos opcionais do protocolo ficam concentrados numa única seam.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `WorkspaceCommandResultDecoder`
- `git diff --check`
