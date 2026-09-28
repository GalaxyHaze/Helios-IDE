# ADR 0142: Callbacks nomeados para code actions LSP

- Status: Accepted
- Date: 2026-09-28

`LspCodeActionRouter` recebia três callbacks posicionais: aplicar um
workspace edit, executar um comando e apresentar o menu. O router tem uma
política coesa para classificar ações e verificar `executeCommandProvider`,
mas o wiring podia associar callbacks com papéis diferentes sem erro de tipo.

A decisão é agrupar os callbacks em `LspCodeActionRouter::Callbacks`, com
campos nomeados. O `QWidget` pai continua um argumento separado porque é um
colaborador de ownership do menu, enquanto os callbacks são políticas
substituíveis de mutação, execução e apresentação. O router continua ligado
diretamente aos resultados específicos de code action do `LspClient`.

## Alternativas

- Manter callbacks posicionais: rejeitado porque a interface não tornava
  visível a semântica de cada `std::function`.
- Criar um adapter para todo o `LspClient`: rejeitado porque duplicaria uma
  interface rasa e não reduziria o conhecimento de protocolo do router.
- Agrupar também o pai do menu em `Callbacks`: rejeitado porque misturaria
  ownership de widget com políticas de comportamento.

## Consequências

- A composição de code actions fica auditável por campo.
- Testes podem substituir cada política independentemente.
- O menu, a aplicação de edits e a execução de comandos continuam em
  módulos/roles distintos.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
