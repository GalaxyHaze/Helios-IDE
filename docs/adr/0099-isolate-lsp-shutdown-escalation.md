# ADR 0099: Isolar a escalada de shutdown do LSP

- Status: Accepted
- Date: 2026-09-27

O shutdown do `LspClient` precisa lidar com servidores que não respondem ao
request `shutdown`: primeiro deve tentar terminar o processo de forma
cooperativa e, se ele continuar vivo, deve forçar a sua terminação. Essa
política estava embutida num callback de `QTimer` e o teste da condição de
processo fazia o ramo `kill()` ser inalcançável enquanto o processo permanecia
vivo.

A decisão é representar a política como o módulo
`LspShutdownEscalation`. Ele não conhece Qt, `QProcess` nem o protocolo LSP;
recebe apenas o facto de o processo estar vivo e devolve a próxima ação
`None`, `Terminate` ou `Kill`. O `LspClient` continua dono do timer e do
transporte, mas deixa a sequência e a idempotência da escalada atrás de uma
interface testável.

## Considered Options

- **Manter a condição no callback do timer**: rejeitado porque a ordem de
  escalada fica implícita e difícil de testar sem processo real.
- **Fazer o transporte decidir entre terminate e kill**: rejeitado porque
  transporte deve expor operações primitivas, não conhecer a política de
  shutdown do protocolo.
- **Criar um controller QObject para todo o lifecycle**: rejeitado nesta
  etapa porque misturaria inicialização, restart e shutdown, criando uma
  interface maior que a seam necessária.
- **Extrair uma política pura de escalada**: escolhido porque concentra a
  decisão real, mantém o lifecycle no `LspClient` e permite teste determinístico.

## Consequences

- Cada sessão envia no máximo uma ação `terminate` e uma ação `kill`.
- O timer continua a aguardar o evento `finished`; a política não assume como
  o transporte executa a ação.
- O lifecycle de inicialização e o restart pendente permanecem no
  `LspClient`, evitando uma abstração prematura.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
- Teste unitário `testLspShutdownEscalationTerminatesThenKillsOnce`

ASan/UBSan não pôde configurar nesta máquina porque a toolchain não fornece
`libasan`/`libubsan`; Valgrind também não está instalado.
