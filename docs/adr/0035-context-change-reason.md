# ADR 0035: Tornar explícita a razão de mudança de contexto

- Status: Accepted
- Date: 2026-09-27

## Context

`ContextManager::contextChanged` informava apenas que o contexto ativo tinha
mudado. O shell tinha de manter flags privadas para distinguir navegação,
criação de um novo contexto e substituição da raiz do contexto atual.

Essa distinção é relevante: navegar ou criar um contexto exige restaurar a
sessão guardada, enquanto trocar apenas a raiz deve preservar os editores
abertos. Sem essa informação na interface, `MainWindow` precisava inferir a
causa através de estado temporário e duplicava a projeção da raiz nos painéis.

## Decision

`ContextManager` passa a emitir `ContextChangeReason` juntamente com
`contextChanged`:

- `Navigation` ao mover-se para um contexto existente;
- `NewContext` ao criar um contexto no fim do histórico;
- `RootChanged` ao trocar a raiz do contexto ativo.

O `ContextManager` continua responsável apenas por armazenar contextos,
índices e notificações. O shell usa a razão para decidir se restaura a sessão;
continua responsável por coordenar runtime, painéis e editor session.

## Alternatives considered

### Manter flags no `MainWindow`

Evitaria alterar o sinal, mas deixaria a causa da mudança implícita e
espalharia invariantes de restauração pelo composition root.

### Emitir sinais diferentes

Sinais separados para navegação, criação e troca de raiz tornariam cada nova
causa uma alteração da superfície de conexão e obrigariam consumidores a
duplicar a atualização comum do contexto.

### Fazer o shell comparar raízes e índices

Seria frágil quando dois eventos tivessem o mesmo root ou quando o histórico
mudasse. A causa é conhecida pelo próprio `ContextManager` no ponto em que o
evento é produzido.

## Consequences

Consumidores recebem uma interface semanticamente completa para tratar
transições de contexto sem conhecer flags do shell. A mudança é um contrato de
sinal que exige atualização explícita dos consumidores, mas remove estado
temporário e atualizações duplicadas de workspace root.

## Verification

`testContextManagerExplainsWhyTheActiveContextChanged` verifica as três causas
e a suíte de build/testes deve permanecer verde após a alteração.
