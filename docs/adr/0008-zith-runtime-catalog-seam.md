---
status: accepted
---

# Zith runtime catalog seam

Installed Zith runtime discovery is separated into `ZithRuntimeCatalog`.
The catalog owns cache paths, release-tag validation, executable/stdlib
validation, newest-release selection, and stale local-cache removal. It does
not perform network requests, launch processes, or emit UI status.

`ZithToolchainManager` remains the asynchronous adapter that coordinates
environment overrides, GitHub requests, downloads, extraction, and signals.
Its existing resolution methods delegate to the catalog while callers migrate
incrementally.

## Considered options

- Keep cache discovery in the network manager: rejected because filesystem
  resolution and network orchestration change for different reasons.
- Replace the manager with a synchronous runtime repository: rejected because
  the current UI depends on cancellable asynchronous downloads and signals.
- Expose raw cache paths to `MainWindow`: rejected because runtime layout is
  an implementation detail of toolchain resolution.

## Consequences

- Release selection can be tested without network access or a Qt event loop.
- Runtime download behavior remains unchanged while the catalog seam is
  introduced.
- Future runtime policies can evolve behind the catalog without expanding
  `MainWindow` or the network adapter.
