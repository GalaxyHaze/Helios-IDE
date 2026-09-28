# ADR 0075: Extrair a sessão de pesquisa Vim

- Status: Accepted
- Date: 2026-09-27

`VimMotionController` misturava a interpretação de motions com uma segunda
política: iniciar uma pesquisa, recolher caracteres, tratar backspace,
confirmar ou cancelar a entrada, procurar de forma circular e repetir a última
pesquisa em qualquer direção. Esses estados eram resetados junto com operações
pendentes, apesar de terem regras e testes próprios.

A decisão é introduzir `VimSearchSession`. O módulo recebe o editor Qt uma vez,
expõe `begin`, `handleKeyPress`, `repeat` e `reset`, e mantém internamente a
sessão ativa, a entrada corrente, a última pesquisa concluída e a direção.
Depois da confirmação, aplica a navegação de pesquisa ao documento; o
`VimMotionController` apenas encaminha eventos e decide quando iniciar ou
repetir a sessão.

## Consequences

- O lifecycle da pesquisa fica localizado numa interface pequena e testável.
- O controlador de motions deixa de conhecer o algoritmo de `QTextDocument::find`
  e os campos de entrada da pesquisa.
- A procura continua deliberadamente ligada ao editor concreto, porque a
  seleção de texto e a posição do cursor são os efeitos observáveis exigidos
  pela interação Vim.
- `VimDocumentOperations` continua responsável por mutações documentais; a
  sessão apenas navega e seleciona resultados.
- Não foi criado um parser genérico de comandos Vim: não há uma segunda
  implementação nem uma variação que justifique essa seam.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
