# ADR 0068: Partilhar o catálogo de atalhos entre superfícies

- Status: Accepted
- Date: 2026-09-27

`SettingsPanel` e `ShortcutsDialog` mantinham cópias idênticas das categorias,
chaves de tradução e sequências de teclas. A decisão é centralizar esse
vocabulário em `ShortcutCatalog`, um módulo sem estado que retorna value
objects; cada widget conserva apenas a montagem da sua árvore e a projeção de
traduções.

A duplicação foi removida porque uma alteração num atalho podia atualizar uma
superfície e deixar a outra divergente. O catálogo é uma seam real: existem
dois adapters de apresentação e a definição deve ser consistente entre eles.
Não foram centralizados estilos ou widgets, pois os dois surfaces têm
responsabilidades visuais próprias.

## Consequences

- Novos atalhos são adicionados num único local.
- Settings e a janela dedicada permanecem livres para evoluir visualmente de
  forma independente.
- O catálogo pode ser testado sem criar widgets ou carregar traduções.
- As sequências exibidas continuam sendo documentação da interface; a
  interpretação efetiva de comandos permanece nas superfícies de shell e
  teclado.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
