# ADR 0101: Extrair o router de requests de features LSP

- Status: Accepted
- Date: 2026-09-27

`LspClient` acumulava a construção de payloads, a escolha dos métodos LSP, a
política de cancelabilidade e o decoding de resultados para completion, hover,
navegação, formatting, symbols, folding, rename, code actions e commands.
Esses métodos eram expostos individualmente, mas repetiam invariantes do
protocolo e tornavam o cliente simultaneamente fachada de sessão e
implementation de todas as features.

A decisão é criar `LspFeatureRequestRouter`. A sua interface interna usa duas
categorias:

- `PositionFeature` para features ancoradas numa posição;
- `DocumentFeature` para features ancoradas no documento.

Rename, code actions, completion resolve e workspace commands permanecem como
operações explícitas porque possuem parâmetros ou políticas distintas. O
router possui a serialização, cancelabilidade e decoding; o `LspClient`
continua a oferecer os métodos históricos como uma fachada de compatibilidade
e encaminha os sinais de resultado.

## Considered Options

- **Manter os requests no `LspClient`**: rejeitado porque mistura lifecycle,
  transporte e feature protocol em um único implementation.
- **Extrair um builder por método LSP**: rejeitado porque criaria módulos rasos
  e espalharia a policy entre muitos ficheiros.
- **Criar um request genérico baseado apenas em strings**: rejeitado porque
  esconderia as diferenças de parâmetros, cancelabilidade e result decoding,
  perdendo segurança semântica.
- **Categorizar features e centralizar a policy num router**: escolhido porque
  reduz conhecimento duplicado, preserva tipos e cria uma seam testável sem
  alterar os callers existentes.

## Consequences

- A superfície de implementação do `LspClient` fica menor, mantendo a sua
  fachada pública estável.
- Mudanças nos payloads ou decoders de features ficam locais ao router.
- `EditorLanguageFeatureController` continua dono da disponibilidade e da
  intenção do usuário; não passa a conhecer detalhes de JSON.
- O router ainda depende do request sender através de um callback, evitando
  acoplamento ao transporte/processo e permitindo teste determinístico.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- Teste `testLspFeatureRequestRouterUsesCategorizedProtocolSeams`
- `git diff --check`
