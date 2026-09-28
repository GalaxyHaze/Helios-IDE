# ADR 0130: Dependências nomeadas para a sessão do editor

- Status: Accepted
- Date: 2026-09-28

`EditorSessionController` recebia nove colaboradores como argumentos
posicionais, seguidos pelos callbacks. A interface exigia que cada call site
memorizasse a ordem exata entre tab widget, serviços LSP, apresentação e
controllers de documentos. Isso tornava a composição frágil: trocar ou
adicionar um colaborador podia compilar com um ponteiro na posição errada.

A decisão é agrupar os colaboradores num valor
`EditorSessionController::Dependencies`, com um campo nomeado para cada
colaborador. O controller continua a possuir a política de lifecycle das tabs
e recebe os mesmos objetos; a mudança é exclusivamente na seam de construção.
Os callbacks continuam separados porque representam eventos que o host deve
adaptar, não dependências persistentes do módulo.

## Alternativas

- Manter argumentos posicionais: rejeitado porque a interface expõe a ordem
  acidental da implementação e não comunica a função de cada dependência.
- Introduzir um `MainWindowContext`: rejeitado porque esconderia ownership e
  transformaria o composition root num objeto global de contexto.
- Criar uma interface abstrata para cada colaborador: rejeitado porque não há
  segunda implementação nem variação real que justifique essas seams.

## Consequências

- A composição documenta os colaboradores por nome e reduz erros de wiring.
- Os testes podem fornecer somente os colaboradores relevantes para cada
  cenário, mantendo a seam concreta e pequena.
- A construção exige algumas linhas de atribuição no composition root, mas
  torna alterações futuras localizadas e revisáveis.
- O comportamento e o ownership do controller permanecem inalterados.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
