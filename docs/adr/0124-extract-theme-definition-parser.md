# ADR 0124: Extrair o parser da definição de tema

- Status: Accepted
- Date: 2026-09-28

`ThemeManager` misturava quatro responsabilidades: localizar ficheiros,
interpretar o schema JSON, manter o tema atualmente ativo e expor cores
semânticas para a aplicação. O parsing também continha os defaults da
palette e a filtragem de cores inválidas, tornando qualquer alteração ao
schema uma alteração no manager global.

A decisão é introduzir `ThemeDefinitionParser`, um módulo sem estado que
converte um `QJsonDocument` numa `ThemeDefinition`. A definição contém apenas
palette, cores customizadas e estilos de syntax. `ThemeManager` continua dono
da descoberta de ficheiros, fallback temporal, estado atual e sinal
`themeChanged`; `ApplicationThemeController` continua dono da aplicação aos
widgets.

## Alternativas

- Manter o parsing em `ThemeManager`: rejeitado porque mistura schema e
  lifecycle num singleton que já é consumido por widgets e highlighters.
- Criar um controlador de temas genérico: rejeitado porque acrescentaria
  dispatch e ownership sem esconder a complexidade do schema.
- Fazer o parser devolver diretamente um `ThemeManager`: rejeitado porque
  impediria testar o schema sem efeitos globais ou sinais Qt.

## Consequências

- O schema JSON, defaults e filtragem de entradas inválidas têm uma única
  seam testável.
- O manager aplica uma definição completa de forma atómica depois de o
  documento ser validado.
- `SyntaxStyle` passa a ser um tipo de modelo partilhado, não um detalhe
  privado do manager.
- A política de preservar o último tema válido permanece no `ThemeManager`;
  o parser não decide quando um fallback deve substituir estado existente.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- teste direto de `ThemeDefinitionParser`
- `git diff --check`
