# ADR 0129: Injetar a política de busca no painel de pesquisa

- Status: Accepted
- Date: 2026-09-28

`SearchPanel` construía o `WorkspaceSearchController` lendo diretamente
`TomlSettingsStore`. Isso fazia o widget conhecer persistência global e
impedia reutilizar o painel com uma política de busca explícita. A dependência
já tinha a forma correta em `WorkspaceSearchController`: um
`ScanPolicyProvider` amostrado no início de cada sessão.

A decisão é fazer `SearchPanel` aceitar esse provider no construtor. O
composition root (`MainWindow`) liga o provider às settings atuais; o painel
apenas encaminha a função para o controller. A dependência é obrigatória no
construtor para evitar uma instância que compila, mas não pesquisa nenhum
ficheiro por usar uma política vazia.

## Alternativas

- Manter a leitura de settings no painel: rejeitado porque mistura UI com
  persistência e dificulta testes/reutilização.
- Fazer `WorkspaceSearchController` ler `TomlSettingsStore`: rejeitado porque
  o controller deve continuar independente de widgets e do armazenamento
  global da aplicação.
- Passar listas de extensões e diretórios em cada busca: rejeitado porque
  espalharia a política e perderia o snapshot coerente por sessão.

## Consequências

- `MainWindow` permanece explicitamente responsável por compor settings com
  busca.
- `SearchPanel` tem uma interface de dependência clara e pode ser exercitado
  com uma política determinística.
- O controller continua a amostrar a política no início da operação, evitando
  misturar settings alteradas no meio de uma busca.
- A interface do painel deixa explícita a dependência necessária para uma
  busca funcional.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cppcheck` e `clang-tidy` no `SearchPanel`; os avisos restantes são
  falsos positivos Qt/MOC e sugestões de estilo genéricas em headers
  existentes.
