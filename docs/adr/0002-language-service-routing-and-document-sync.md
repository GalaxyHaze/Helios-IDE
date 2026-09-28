---
status: accepted
---

# Language service routing and document synchronization

Helios derives a document's language identity from its file suffix and routes Zith and C-family documents to separate `LspClient` instances; plain-text documents do not use a language service. The shared `LspClient` implementation owns protocol transport and request handling, while `ClangdLifecycleCoordinator` owns clangd process policy and `LspDocumentCoordinator` owns document binding and synchronization.

The suffix classification and stable LSP identifiers are implemented by the
`LanguageIdentity` module. `MainWindow` adapts these modules to the Qt editor
session and remains responsible for settings discovery, UI presentation, and
reconnecting open documents after a client initializes. This keeps identity,
lifecycle policy, and document synchronization testable without constructing
the full window.

## Considered options

- Duplicate protocol implementations for Zith and clangd: rejected because framing, request tracking, timeouts, and document protocol behavior are shared.
- One client process for every open document: rejected because the current language services are workspace-oriented and already support multiple documents.

## Consequences

- Language routing must remain deterministic for a given path.
- Moving a document between language identities requires closing it on the previous client before opening it on the new client.
- Lifecycle policy is isolated in `ClangdLifecycleCoordinator`; future
  language services should use an explicit lifecycle seam rather than adding
  another process-policy branch to `MainWindow`.
- Language identity changes should be made through `LanguageIdentity` and its
  focused tests rather than by adding another suffix switch to `MainWindow`.
