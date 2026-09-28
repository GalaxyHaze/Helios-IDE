# ADR 0023: Representar a identidade do runtime Zith como value object

- Status: Accepted
- Date: 2026-09-27

## Context

`MainWindow` mantinha a identidade do runtime Zith em várias strings
independentes: tag, caminho do `zith-lsp`, caminho da stdlib e workspace
associado. O mesmo shell também mantinha o texto de status apresentado nas
definições e no gestor de LSP.

Essa combinação permitia que uma transição limpasse apenas parte da
identidade, ou que uma mensagem de apresentação apagasse acidentalmente os
dados necessários para reiniciar o cliente. O ciclo de vida do runtime ainda
pertence ao shell, mas a sua identidade e a comparação de runtime ativo são
um conceito coeso.

## Decision

`ZithRuntimeState` representa a identidade materializada de um runtime com:

- tag da release;
- caminho do executável LSP;
- caminho da stdlib;
- workspace ao qual o runtime foi associado.

O módulo também mantém o texto de status, mas trata-o como estado de
apresentação independente da identidade. `activate()` substitui a identidade,
`matches()` compara os campos necessários para reutilizar um cliente e
`clearRuntime()` limpa apenas a identidade. Assim, uma mensagem como
“Runtime cache cleared” ou “Frontend error” pode sobreviver a uma limpeza ou
troca de runtime sem corromper os dados usados pelo ciclo de vida.

`MainWindow` continua a decidir quando resolver, iniciar, parar ou reiniciar
o runtime. `ZithRuntimeState` não é um coordenador de processos nem conhece
widgets, settings ou `LspClient`.

## Alternatives considered

### Manter strings independentes no `MainWindow`

Evitaria uma classe, mas deixaria invariantes de identidade espalhadas pelos
handlers de enable/disable, cache, restart e erro. Cada novo caminho teria de
lembrar quais campos limpar ou preservar.

### Criar um coordenador completo de runtime imediatamente

Daria ao módulo responsabilidade sobre resolução, processos, cache,
apresentação e restart ao mesmo tempo. Isso criaria uma interface larga antes
de haver uma política estável e misturaria o runtime Zith com o lifecycle de
clangd.

### Limpar também o status em `clearRuntime()`

Seria semanticamente simples, mas perderia a mensagem que explica a transição
em curso. A apresentação deve poder dizer que o runtime foi limpo enquanto a
identidade antiga já não é reutilizável.

## Consequences

O shell tem uma fonte única para a identidade do runtime e deixa de comparar
ou limpar quatro strings manualmente. Restart e reutilização de cliente
passam a depender de uma interface pequena e testável.

O status continua no mesmo value object por enquanto, embora seja uma
preocupação de apresentação. Se os consumidores crescerem, essa separação
poderá ser extraída para um modelo de transição ou adapter de apresentação
sem alterar a identidade do runtime.

## Verification

`testZithRuntimeStateTracksIdentitySeparatelyFromStatus` verifica ativação,
comparação, limpeza da identidade e preservação independente do texto de
status.
