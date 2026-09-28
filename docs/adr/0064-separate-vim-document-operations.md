# ADR 0064: Separar operações documentais da máquina Vim

- Status: Accepted
- Date: 2026-09-27

## Context

`VimMotionController` acumulava a interpretação de sequências de teclas, o
estado de modos e pending operations, a busca, e a implementação de todas as
mutações sobre `QPlainTextEdit`. Isso fazia o controller ser simultaneamente
uma máquina de estados e uma camada de edição textual.

As mutações tinham uma semântica própria: movimento, operações sobre linhas,
seleções visuais, clipboard, paste, replace e abertura de linhas. Elas não
precisam conhecer o motivo da operação nem o estado de comandos que levou até
elas.

## Decision

Criar `VimDocumentOperations` como adapter estreito sobre o editor. Ele
expõe operações semânticas para:

- mover o cursor;
- abrir linhas;
- aplicar delete/change/yank a uma seleção;
- operar sobre linhas;
- operar sobre seleção visual;
- apagar caracteres, fazer paste e substituir caracteres.

`VimMotionController` continua dono de:

- interpretação de `QKeyEvent`;
- modos Off, Normal e Insert;
- pending operations e contagens;
- estado de find, search, command line e visual mode;
- transições de modo e signals `modeChanged`/`commandEntered`.

`VimPendingOperation` foi movido para um header de domínio pequeno, e
`VimMotionController::PendingOp` permanece como alias para preservar a
interface nominal existente.

O adapter não decide transições de modo nem emite comandos; quando uma
operação de change requer Insert, o controller interpreta o resultado e faz a
transição.

## Alternatives considered

### Manter mutações no controller

Rejeitado porque mistura a máquina de estados com detalhes de `QTextCursor`,
clipboard e edição de blocos, tornando cada nova operação Vim mais difícil de
localizar e testar.

### Criar um parser separado para cada tecla Vim

Rejeitado neste estágio porque o estado de sequências, buscas e modos ainda
forma uma política única. Dividir por tecla produziria interfaces rasas e
duplicaria transições.

### Fazer o adapter possuir os modos Vim

Rejeitado porque isso faria uma camada de mutação textual conhecer input e
presentation state. O mesmo adapter deve poder ser chamado por uma futura
interface de comandos sem importar a máquina de estados atual.

## Consequences

As operações documentais podem ser testadas com um editor Qt pequeno sem
simular sequências completas de comandos. A máquina Vim fica mais localizada e
preserva os seus signals e a API pública. O adapter continua Qt-facing por
necessidade, mas não conhece `QKeyEvent`, modos ou comandos.

O alias de `PendingOp` evita uma quebra imediata para consumidores existentes;
novas interfaces devem preferir `VimPendingOperation` quando precisarem
referenciar o value type fora do controller.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `testVimMotions`
- `git diff --check`
