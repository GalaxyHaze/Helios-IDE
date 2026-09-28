# ADR 0123: Extrair a sessão de comandos Vim

- Status: Accepted
- Date: 2026-09-28

O `VimMotionController` tratava diretamente o modo `:`: mantinha o texto
parcial, editava com Backspace, decidia entre cancelamento e submissão e
normalizava o comando antes de o emitir. Essa é uma sessão transitória
independente da interpretação posterior de comandos como `w`, `wq` ou `q`.

A decisão é introduzir `VimCommandSession`, um módulo sem dependência do
editor que possui esse ciclo de vida. O controller inicia e reinicia a sessão,
consome o comando concluído e continua responsável por emitir
`commandEntered`; a interpretação da ação continua fora desta seam.

## Alternativas

- Manter o estado no `VimMotionController`: rejeitado porque mistura mais uma
  máquina de entrada com dispatch de movimentos e operações documentais.
- Criar um interpretador de comandos Vim: rejeitado neste passo porque
  interpretação de ações é responsabilidade da camada de interação da IDE,
  não da sessão de edição do texto.

## Consequências

- A entrada `:` pode ser testada sem `QPlainTextEdit` nem o controller.
- Cancelamento, edição, normalização e entrega única do comando têm uma
  implementação local.
- O controller conserva a política de modo e a emissão do sinal, evitando uma
  abstração genérica de comandos.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `VimCommandSession`
- `git diff --check`
