---
status: accepted
---

# Zith runtime resolution policy

When resolving the Zith runtime, Helios first honors a complete and valid pair
of environment overrides. Otherwise it removes stale local cache entries,
can expose the newest installed release immediately when cached use is
preferred, and then checks the latest remote release. If remote resolution or
installation fails, it falls back to the newest valid installed release.

## Consequences

- Environment overrides are all-or-nothing: a partial pair is ignored.
- Local development checkouts are never guessed from host-specific paths;
  development uses the explicit environment overrides.
- A cached runtime may be usable before the remote update check completes.
- Runtime resolution can emit an immediately usable result and later discover a newer release.
- The runtime manager owns fallback and cache policy; UI modules should consume status, ready, and failed outcomes rather than reproduce precedence rules.
