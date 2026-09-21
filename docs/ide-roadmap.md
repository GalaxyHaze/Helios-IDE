# Roadmap de evolucao da IDE

## Estado atual

- O editor base esta funcional: multi-tabs, syntax highlighting, snippets, find/replace por ficheiro, busca no workspace, arvore de ficheiros, painel Git com stage/unstage/commit e preferencias persistentes.
- Os Vim motions foram expandidos com operadores de edicao, busca, visual mode e `:w`, `:q`, `:wq`.
- O painel inferior e as traducoes EN/PT foram sincronizados nas acoes de limpar e fechar.
- O cliente LSP ja negocia muitas capabilities e tem code para completion, hover, definition, declaration, implementation, references, highlight, signature help, document symbols, formatting, folding, rename, code actions e semantic tokens.
- A integracao LSP existe no `MainWindow` e no `CodeEditor`, mas precisa de ser re-validada de ponta a ponta com um `zith-lsp` real e com um conjunto de testes automaticos maior.

## Ordem de prioridade

1. **Reintegrar e testar o LSP** antes de novos recursos pesados.
2. **Estabilizar as funcionalidades ja anunciadas** no README e no UI.
3. **Aumentar a cobertura de testes automaticos** enquanto se adiciona cada feature.
4. **Refinar UX e localizacao** depois de a base funcional estar estavel.

## Fase 1: LSP reintegrado e testado

- Instalar/preparar um `zith-lsp` local e definir os passos manuais de verificacao.
- Confirmar o fluxo `initialize`, `initialized`, `didOpen`, `didChange` e `didClose`.
- Confirmar que o runtime bootstrap (cache / online) nao quebra em setup limpo e em setup com cache existente.
- Testar manualmente os seguintes recursos com pelo menos um projeto Zith:
  - completion (incluindo snippet e resolve)
  - hover
  - go to definition / declaration / implementation
  - find usages e document highlights
  - signature help ao abrir parenteses
  - diagnostics apos edicao e apos `publishDiagnostics`
  - format document
  - rename symbol
  - outline / document symbols
  - build/check/run/stop via `workspace/executeCommand`
  - `zith/requestSaveAll`, `zith/frontendStatus` e `zith/metrics`
- Escrever testes automaticos para:
  - parse de respostas `completion`, `hover`, `definition`, `references`, `signatureHelp`, `formatting`, `rename`, `documentSymbols`, `foldingRanges`, `codeActions` e `semanticTokens`
  - cancelamento de pedidos obsoletos por versao de documento
  - respostas LSP com `error`
  - frames LSP com headers parciais/fragmentados
  - mensagens `window/logMessage`, `window/showMessage`, `publishDiagnostics`, progress e `zith/*`
  - aplicacao real de `WorkspaceEdit` em ficheiros locais (renome/refactor)
- Criar um harness de LSP falso em teste que simule um servidor simples de stdin/stdout.
- Atualizar `docs/zith-lsp-api-requirements.md` se algum comportamento real divergir da API.

## Fase 2: corrigir lacunas da integracao que ja existe

- Ligar semantic tokens ao destaque do editor e validar atualizacao incremental.
- Ligar folding ranges ao editor (fold/unfold das regioes).
- Garantir que `OutlinePanel` reflete mudancas do documento e datasource de document symbols sem ficar com estado antigo.
- Confirmar que alteracoes feitas por Vim motions sao enviadas ao LSP com a versao correta.
- Confirmar que `Ctrl+Z` / `Ctrl+Shift+Z` do editor continua a funcionar ao lado dos atalhos Vim.
- Melhorar a UI de erros do LSP: mostrar mensagem de arranque fracassado, path em falta e timeout.
- Adicionar um botao ou comando de `LSP: reiniciar e reabrir documentos abertos` para recuperar de crashes.
- Revisar cancelamento de requests: usar `$/cancelRequest` tambem quando se troca de linha/editor.

## Fase 3: busca, navegacao e edicao

- Mover a busca no workspace para ser asincrona e com cancelamento.
- Adicionar filtros de extensao, case-sensitive, regex e exclusao de diretorios ja persistidos nas definicoes.
- Abrir resultados de busca com o termo destacado e foco no editor.
- Adicionar multi-cursor / selecao de blocos em coluna.
- Adicionar fold regions e minimap apenas depois de o LSP/editor base estar estavel.
- Concluir o suporte de undo/redo correcto para operadores Vim (`dd`, `yy`, `cc`, `x`, `r`).

## Fase 4: Git e workspace

- Diff por ficheiro com highlighting.
- Checkout de branches e criar branch novo.
- Publish / sync com remoto.
- Estado de merge/rebase e resolucao de conflitos.
- Fechar ficheiros com unsaved changes pedindo confirmacao consistente.
- Melhorar deteccao de project root e recarregamento da arvore.

## Fase 5: UX, temas e i18n

- Unificar headers, botoes e lista de paineis laterais.
- Centralizar cores e espacamentos num unico sistema visual.
- Revalidar todas as strings EN/PT e ligar mais textos estaticos ao `TranslationManager`.
- Adicionar chaves de localizacao para dialogos de confirmacao e mensagens de status ainda hardcoded.
- Aplicar tema a paineis novos sem depender de stylesheets duplicadas.

## Criterios de aceite para uma iteracao

- `cmake --build build --target Helios test_helios -j` termina sem erros.
- `ctest --test-dir build --output-on-failure` passa.
- `git diff --check` passa antes de commit.
- Para mudancas LSP: o fluxo manual com um projeto Zith real funciona do arranque ate editacao, diagnostico e navegacao.
- Para mudancas de UI: validar em pelo menos um tema claro e um escuro e nos dois idiomas disponiveis.

## Notas de risco

- O projecto nao tem um servidor LSP Zith de referencia sempre disponivel nos testes; os testes feitos com um servidor simulado nao provam compatibilidade total.
- Algumas responses LSP (semantic tokens delta, workspace folders, code lens) ainda nao sao contracto suportado.
- Alteracoes de Vim podem interferir com atalhos globais do editor; sempre revalidar atalhos existentes.
