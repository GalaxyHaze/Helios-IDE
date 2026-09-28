# ADR 0061: Separar a interação de language features do documento

- Status: Accepted
- Date: 2026-09-27

## Context

`CodeEditor` acumulava duas responsabilidades diferentes: manter o documento
editável e interpretar a interação local com o language service. A mesma classe
mantinha timers de hover e document highlights, ligações de vários resultados
LSP, atalhos de navegação e completion, Ctrl-click e parte da política de
aceitação de resultados assíncronos.

Essa combinação fazia a superfície do editor crescer sempre que uma nova
language feature era adicionada. Também tornava difícil verificar a fronteira
entre estado do documento e comportamento dependente de um cliente LSP, em
especial durante detach, troca de documento e chegada de resultados antigos.

O objetivo não é transformar `CodeEditor` num widget passivo. O documento
continua a ser o dono da identidade e do estado que outras partes da aplicação
já consomem. A decisão é extrair uma seam profunda para a política de
interação language-aware, preservando a interface pública existente.

## Decision

Criar `EditorLanguageFeatureController` como módulo filho de `CodeEditor`. Ele:

- mantém as ligações de diagnostics, hover, definition, implementation,
  declaration, signature help e document highlights;
- mantém os timers de hover e highlights;
- interpreta os atalhos de formatting, navigation, completion e signature
  help;
- interpreta Ctrl-click para navegação;
- solicita as language features através do `LspClient` anexado;
- rejeita resultados dirigidos a outra URI ou a uma versão documental
  incompatível quando essa política já era exigida pelo editor;
- interrompe timers e desconecta todos os resultados ao fazer detach.

`CodeEditor` continua a possuir:

- o documento Qt e o texto editado;
- path, URI e versão documental;
- diagnostics armazenados e a sua renderização;
- sincronização documental com o language service;
- completer, seleção e demais superfícies visuais;
- sinais públicos, incluindo `navigateToLocation`;
- a interface pública existente para os consumidores do editor.

A seam privada usa `friend class EditorLanguageFeatureController`; isso evita
alargar a API pública apenas para permitir a extração e mantém a composição
local ao editor.

## Alternatives considered

### Manter toda a política em `CodeEditor`

Rejeitado porque preserva o monólito e mistura invariantes do documento com
interação opcional de language service. Cada nova feature aumentaria o custo
de navegação e o risco de duplicar regras de detach ou versionamento.

### Criar controllers separados para cada feature

Rejeitado neste estágio porque dividir completion, hover e navigation em
controllers sem uma política comum produziria interfaces rasas e duplicaria
temporizadores, acesso ao client e validações de URI/version. A unidade atual
é a interação language-aware do documento, não cada request individual.

### Expor getters e mutators públicos adicionais em `CodeEditor`

Rejeitado porque aumentaria o acoplamento dos consumidores e transformaria
detalhes de composição numa API permanente. A seam privada é suficiente para o
controller e deixa a superfície externa estável.

## Consequences

`CodeEditor` fica menor e conserva as responsabilidades que definem o
documento. A interação LSP passa a ser localizável, testável e substituível sem
alterar os consumidores do editor. O controller ainda depende de detalhes
privados do editor; isso é deliberado, porque a seam representa uma única
política de um único documento, não um serviço LSP reutilizável globalmente.

O próximo passo de evolução é testar explicitamente resultados obsoletos,
navigation emitida e eventos recebidos depois do detach. Não se deve extrair
um controller adicional até existir uma segunda política coerente com fronteira
própria.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cppcheck` e `clang-tidy` no novo controller e em `Code.cpp`; os avisos
  observados são os padrões existentes do projeto/Qt, sem erro novo bloqueante.
