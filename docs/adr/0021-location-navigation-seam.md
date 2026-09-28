# ADR 0021: Centralizar navegação para localizações

- Status: Accepted
- Date: 2026-09-27

## Context

Editor, painel de diagnósticos, outline, painel de referências e busca tinham
callbacks independentes que repetiam a mesma sequência: converter uma URI ou
caminho, abrir o ficheiro e posicionar o editor ativo. Além da duplicação,
cada origem podia tratar URI não local de forma diferente.

## Decision

`LocationNavigator` é o seam único para navegação baseada em localização. Ele
expõe `navigateToPath` e `navigateToUri`, recebe callbacks para abrir o
ficheiro e obter o editor atual, e só posiciona o editor depois da abertura.
URIs locais e caminhos sem esquema são aceitos; esquemas remotos são
rejeitados sem chamar o callback de abertura.

O módulo também funciona como adapter dos sinais de `CodeEditor`,
`DiagnosticsPanel`, `OutlinePanel`, `ReferencesPanel` e `SearchPanel`,
mantendo essas origens livres da política de abertura e posicionamento. A
seleção de um símbolo no outline é uma navegação local dentro do editor atual:
ela reutiliza o mesmo seam, mas não abre um novo ficheiro.

## Alternatives considered

### Manter callbacks em cada origem

Parecia simples, mas mantinha quatro cópias de uma política com efeitos
visíveis e permitia divergência na interpretação de URIs.

### Fazer `MainWindow` expor apenas `navigateToLocation`

Isso reduziria a duplicação local, mas manteria a política dentro do shell e
impediria testar URI inválida sem construir a janela completa.

### Permitir qualquer URI através de `QUrl::toLocalFile`

Seria permissivo demais: uma localização remota poderia ser transformada em
um caminho vazio e atingir a abertura de ficheiros. O router rejeita
explicitamente esquemas não locais.

## Consequences

As origens compartilham uma única política e o shell só injeta abertura e
seleção do editor ativo. O módulo depende de widgets apenas para adaptar
sinais e posicionar o editor, enquanto a decisão de localidade permanece
testável através de uma interface pequena.

## Verification

`testLocationNavigatorNormalizesLocalUrisAndRejectsRemoteTargets` verifica
normalização de URI local, abertura, posicionamento e rejeição de URI remota.
`testLocationNavigatorAttachesOutlineSelection` verifica que a seleção de um
símbolo posiciona o editor atual através do adapter do módulo.
