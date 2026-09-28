# ADR 0134: Dependências nomeadas para a apresentação do workspace

- Status: Accepted
- Date: 2026-09-28

`EditorWorkspacePresentationController` recebia seis widgets em posições
fixas. Embora todos fossem tipos diferentes, a lista escondia a relação entre
tab widget, stack central, welcome state e find bar, tornando a composição
mais difícil de revisar quando a shell mudava.

A decisão é agrupar esses widgets em
`EditorWorkspacePresentationController::Dependencies`. O controller continua
responsável pela projeção da sessão no workspace visual e pela política da
find bar; não passa a possuir os widgets nem recebe novas regras de domínio.

## Alternativas

- Manter os argumentos posicionais: rejeitado porque a composição não
  comunicava a função de cada widget.
- Criar um contexto geral da janela: rejeitado porque esconderia ownership e
  ampliaria o seam além da apresentação do workspace.
- Extrair cada operação da find bar: rejeitado porque as operações formam uma
  política coesa e já têm uma interface pequena.

## Consequências

- A composição do shell fica legível campo a campo.
- Testes podem montar a superfície visual explicitamente.
- O controller mantém a mesma responsabilidade e comportamento.
- A interface não unifica superfícies visuais que têm lifecycle diferente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
