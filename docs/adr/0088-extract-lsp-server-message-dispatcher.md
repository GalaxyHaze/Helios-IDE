# ADR 0088: Extrair o dispatch de mensagens iniciadas pelo servidor LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` acumulava duas políticas de entrada diferentes: respostas a
requests que o cliente iniciou e mensagens iniciadas pelo servidor. A segunda
incluía requests que exigem resposta, diagnostics sujeitos à versão do
documento, notificações LSP de UI e extensões Zith de progresso, métricas e
processos.

A decisão é introduzir `LspServerMessageDispatcher`. O módulo recebe três
dependências estreitas: envio de uma resposta JSON-RPC, estado de execução do
processo e validação da versão documental. A sua interface pública é apenas
`dispatch`; os eventos traduzidos são publicados como signals. O `LspClient`
continua dono das respostas dos requests que iniciou, do request tracker, do
lifecycle do processo e da interface pública consumida pelo editor.

## Considered Options

- **Manter o dispatch no `LspClient`**: rejeitado porque cada nova notificação
  do servidor aumentaria um cliente já responsável por lifecycle e requests.
- **Criar um router genérico de mensagens**: rejeitado porque teria apenas
  forwarding de callbacks e duplicaria a política que precisa ser testada.
- **Extrair builders individuais para cada notificação**: rejeitado porque
  espalharia a classificação, a regra de versões stale e as respostas JSON-RPC.
- **Extrair `LspServerMessageDispatcher`**: escolhido porque concentra a
  política completa de mensagens iniciadas pelo servidor atrás de uma operação
  pequena e testável sem `QProcess`.

## Consequences

- `LspClient` deixa de conhecer a implementação de diagnostics, progress,
  mensagens de janela e extensões de runtime vindas do servidor.
- A filtragem de diagnostics stale é testada junto do dispatch que a aplica,
  sem iniciar um servidor LSP.
- O dispatcher não conhece `LspClient`, widgets, capabilities ou requests
  pendentes; recebe apenas callbacks que representam as dependências reais.
- A interface pública de `LspClient` permanece estável, pois ele reexpõe os
  eventos através dos seus signals existentes.

## Verification

- `cmake --build build -j 2`
- teste direto de `LspServerMessageDispatcher`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
