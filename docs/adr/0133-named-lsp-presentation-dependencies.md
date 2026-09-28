# ADR 0133: Dependências nomeadas para a apresentação do runtime LSP

- Status: Accepted
- Date: 2026-09-28

`LspRuntimePresentationController` recebia sete destinos e fontes de estado
como argumentos posicionais, além de três callbacks. A interface misturava
superfícies de apresentação, lifecycle e políticas de consulta numa lista
difícil de auditar; uma alteração podia ligar um destino à posição errada sem
alterar tipos.

A decisão é agrupar os colaboradores persistentes em
`LspRuntimePresentationController::Dependencies`. Os callbacks de enablement
e resolução de clangd continuam argumentos separados porque são políticas
fornecidas pelo composition root. O controller continua a projetar o mesmo
estado para `SettingsPanel`, `LspManagerDialog`, log e status bar; esta
decisão não tenta unificar as duas superfícies visuais, que têm densidades e
controles diferentes.

## Alternativas

- Manter a lista posicional: rejeitado pelo risco de wiring silencioso.
- Criar um widget LSP compartilhado: rejeitado porque o painel lateral e o
  diálogo têm hierarquias, controles e densidades diferentes.
- Agrupar dependências e callbacks num contexto único: rejeitado porque
  confundiria colaboradores possuídos com políticas do host.

## Consequências

- A composição fica legível campo a campo.
- A mesma apresentação semântica continua reutilizável em superfícies
  visuais distintas.
- Os testes podem montar apenas os colaboradores necessários.
- O composition root ganha algumas atribuições explícitas, mas as mudanças de
  wiring ficam localizadas.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
