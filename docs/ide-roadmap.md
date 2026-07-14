# Roadmap de evolucao da IDE

## Estado atual

- A base da IDE ja oferece editor com LSP, diagnosticos, arvore de arquivos, busca no workspace, painel Git inicial e preferencias basicas.
- O painel Git ainda estava em modo de leitura: mostrava `git status`, mas nao executava fluxo de stage, unstage ou commit.
- A barra lateral de atividades tem a estrutura certa, mas a organizacao visual ainda estava inconsistente.

## Objetivos

1. Atualizar a IDE sem quebrar a base atual de edicao, LSP e multiplos contextos.
2. Tirar recursos "stubados" do papel, com prioridade para Git.
3. Refinar a aparencia para que os paineis parecam parte do mesmo produto.

## Fase 1: estabilizacao e limpeza

- Centralizar temas, cores e espacamentos hoje espalhados por varios widgets.
- Remover caminhos absolutos hardcoded para LSP, snippets e arquivos de exemplo.
- Revisar o fluxo de bootstrap da janela principal para separar inicializacao de servicos, layout e comandos.

## Fase 2: recursos que ainda sao stubs

### Git

- Entregar fluxo minimo completo:
  - resumo do repositorio e branch atual
  - lista de arquivos alterados
  - `stage all`
  - `stage selected`
  - `unstage selected`
  - `commit` com mensagem
- Depois disso:
  - diff por arquivo
  - checkout de branches
  - publish/sync com remoto
  - conflitos e estado de merge/rebase

### Busca e navegacao

- Mover busca do workspace para execucao assincrona.
- Adicionar filtros por extensao e opcao de case-sensitive/regex.
- Permitir abrir resultado ja com destaque do termo encontrado.

### Configuracoes

- Persistir preferencias do editor entre sessoes.
- Expor tema, familia de fonte e toggles de comportamento do editor.

## Fase 3: aparencia

- Unificar cabecalhos, botoes e listas dos paineis laterais.
- Melhorar a `ActivityBar` com hierarquia visual mais clara entre navegacao principal e configuracoes.
- Revisar status bar, tabs e breadcrumbs para usar a mesma linguagem visual.

## Progresso desta iteracao

- painel Git evoluido de leitura para operacao basica de stage/unstage/commit
- `ActivityBar` reorganizada para separar acoes principais de configuracoes
- preferencias de fonte e quebra de linha agora persistem entre sessoes

## Validacao minima por entrega

- `cmake --build build`
- abrir `./build/Helios`
- validar manualmente:
  - abrir repositorio Git
  - fazer alteracao em arquivo
  - usar stage/unstage no painel Git
  - executar commit com mensagem
  - alternar Explorer/Search/Git/Settings e verificar layout
