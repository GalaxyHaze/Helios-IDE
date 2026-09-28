# ADR 0057: Restaurar sessões de forma resiliente a ficheiros ausentes

- Status: Accepted
- Date: 2026-09-27

## Context

`EditorSessionState` guarda caminhos que podem deixar de existir entre duas
execuções da IDE. A restauração já ignorava um caminho que não pudesse ser
aberto, mas aplicava depois `currentTab` diretamente ao índice das tabs
materializadas. Quando um ficheiro anterior ao ativo desaparecia, esse índice
passava a apontar para outro documento.

O mesmo problema aparece quando o próprio ficheiro ativo foi removido. A
sessão precisa de uma seleção determinística sem transformar a restauração
numa operação interativa.

## Decision

`EditorSessionController::restoreState()` reconstrói as tabs na ordem dos
ficheiros persistidos e relaciona o índice persistido com a tab que foi
materializada com sucesso. O ficheiro ativo é preservado quando existe e pode
ser aberto. Se o ficheiro ativo estiver ausente ou falhar a leitura, a
primeira tab restaurada torna-se ativa. Se nenhum ficheiro puder ser
restaurado, a sessão fica sem tabs.

Ficheiros ausentes não produzem diálogos nem alteram o formato persistido.
A política de identidade de `openFilePath()` continua a impedir tabs
duplicadas para o mesmo caminho.

## Alternatives considered

### Aplicar diretamente `currentTab` depois da restauração

Rejeitado porque os índices persistidos e materializados deixam de coincidir
quando um ficheiro falha ou quando a política de deduplicação elimina uma tab.

### Remover previamente os caminhos inválidos do estado persistido

Rejeitado porque a restauração deve ser tolerante sem fazer I/O de mutação
durante o carregamento; a próxima captura de sessão pode atualizar o estado
naturalmente.

### Pedir ao utilizador para localizar cada ficheiro ausente

Rejeitado porque transforma o arranque numa operação modal e não é necessário
para recuperar os documentos que continuam disponíveis.

## Consequences

A ordem dos ficheiros disponíveis é preservada e a seleção ativa deixa de
depender de falhas de leitura anteriores. O utilizador não é avisado sobre
ficheiros ausentes nesta etapa; uma futura superfície de recuperação pode
expor essa informação sem alterar a regra de seleção.

## Verification

Os testes de sessão cobrem um ficheiro ausente antes do ficheiro ativo e o
caso em que o próprio ficheiro ativo está ausente. A suite CMake/CTest valida
também a restauração normal e a captura do índice persistível.
