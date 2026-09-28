# ADR 0054: Centralizar a ativação de roots de workspace

- Status: Accepted
- Date: 2026-09-27

## Context

A seleção e ativação de um workspace aconteciam em mais de um caminho do
shell. Abrir uma pasta substituía o root do contexto atual, criar um projeto
adicionava um contexto, e `Alt+Right` adicionava um contexto diretamente
através de `ContextManager`. Esses caminhos repetiam parte da política de
salvar o contexto atual e atualizar projetos recentes. O caminho de navegação
também podia alterar o histórico sem passar pela atualização de recents e do
Welcome. O startup ainda restaurava o último root chamando `ContextManager`
diretamente, sem validar que o caminho era um diretório e sem expressar a
diferença entre restauração persistida e ativação iniciada pelo utilizador.

Além da duplicação, a validação do diretório estava misturada com a escolha
visual do diretório. Isso tornava difícil provar que um caminho inválido não
produzia efeitos parciais.

## Decision

Um módulo de ativação de root expõe operações semânticas distintas:

- substituir o root do contexto atual;
- criar um novo contexto para um root.
- restaurar o root persistido do contexto atual.

O módulo valida e normaliza o diretório antes de executar qualquer callback ou
mutação. As duas operações de ativação iniciadas pelo utilizador salvam o
contexto ativo, alteram o `ContextManager` e atualizam
persistência/apresentação de projetos recentes através de adapters explícitos.
A restauração persistida usa a mesma validação e mutação de root, mas não salva
o contexto nem reordena recents: ler estado não deve produzir efeitos de
ativação.

`WorkspaceRootInteractionController` é responsável por abrir os diálogos,
escolher o texto visual e definir o diretório inicial. Ele delega a operação
de substituir ou criar ao módulo de ativação. `ContextNavigationController`
decide quando pedir um novo root durante a navegação, mas delega a criação do
contexto ao módulo de interação em vez de mutar `ContextManager` diretamente.

## Alternatives considered

### Manter a política em `MainWindow`

Rejeitado porque deixa a criação de contexto da navegação com uma segunda
implementação e mantém validação, persistência e atualização do Welcome
espalhadas pelo shell.

### Reutilizar ativação completa durante o startup

Rejeitado porque restaurar o último root não é uma ação nova do utilizador.
Salvar novamente o contexto e reordenar a lista de recents durante a leitura
do estado seria um efeito lateral e poderia mascarar a diferença entre
restauração e ativação.

### Fazer `ContextNavigationController` chamar `ContextManager`

Rejeitado porque o histórico passaria a ter um caminho de mutação que não
conhece a política de salvar contexto nem a atualização de projetos recentes.

### Criar um coordenador genérico de workspace

Rejeitado porque misturaria seleção de diretório, contexto, painéis e
persistência num módulo com interface mais larga e menor profundidade. A
seam deve representar a decisão específica de ativar um root.

## Consequences

Todos os caminhos de entrada usam a mesma política de seleção e validação. As
ações do utilizador também compartilham ordenação e persistência; a restauração
persistida explicitamente não as executa. Um root inválido ou uma seleção
cancelada não salva o contexto nem atualiza recents em nenhum dos caminhos.
`MainWindow` compõe os dois módulos, mas deixa de conter a política dos
diálogos e os detalhes da mutação de contexto.

O módulo usa adapters para persistência e refresh do Welcome, mantendo a
política testável sem criar janelas ou acessar o armazenamento global nos
testes.

## Verification

`testWorkspaceRootControllerRejectsInvalidRootsWithoutEffects` verifica a
ausência de efeitos para um diretório inválido.

`testWorkspaceRootControllerCentralizesRootActivationPolicy` verifica
normalização, ordenação de salvar antes da mutação, persistência de recents e
as diferenças entre substituir o root e criar um contexto.

`testWorkspaceRootControllerRestoresPersistedRootWithoutActivationEffects`
verifica que o startup pode restaurar apenas um diretório válido sem salvar ou
alterar recents, e rejeita um caminho persistido que aponta para um ficheiro.

`testContextNavigationControllerPreservesHistoryPolicy` verifica que a
navegação solicita a criação de contexto através do seam, sem mutar o
`ContextManager` diretamente.

`testWorkspaceRootInteractionControllerCentralizesDirectorySelection`
verifica a política partilhada de títulos, diretórios iniciais e operações de
ativação sem abrir diálogos reais.
