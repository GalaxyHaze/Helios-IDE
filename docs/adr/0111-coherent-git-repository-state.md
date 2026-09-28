# ADR-0111: Publish a coherent Git repository state

- Status: Accepted
- Date: 2026-09-27

## Context

`GitRepositorySession` already owned Git process sequencing, parsing, refresh
after mutations, and busy state, but exposed those facts through several
correlated signals: status, repository availability, remote availability, and
busy. Consumers had to reconstruct one repository state from events that were
published separately.

That interface made a new adapter easy to implement incorrectly. A panel could
observe a status snapshot before remote availability, or retain rows while the
repository had become unavailable. The session was already the legitimate
owner of this invariant; the missing piece was a coherent value crossing its
seam.

## Decision

Introduce `GitRepositoryState` with:

- active root path;
- branch and status entries;
- repository availability;
- `origin` remote availability;
- busy state.

`GitRepositorySession::state()` returns the latest value and
`stateChanged` publishes a complete snapshot whenever one of those facts
changes. `GitPanel` consumes this state as its primary adapter interface.

The former correlated state signals were removed after auditing all consumers.
`messageChanged` and `commitSucceeded` remain because they describe operation
outcomes rather than fields of repository state.

`GitRepositorySession` continues to own process lifecycle, operation
serialization, parsing, automatic refresh, and error classification.
`GitPanel` continues to own selection, row construction, theme, dialogs, and
user-facing interaction.

## Alternatives considered

### Keep correlated signals as the only interface

Rejected because consumers must reconstruct state and reason about signal
ordering. That distributes the repository invariant across adapters.

### Make `GitPanel` poll every getter

Rejected because polling does not define when a coherent transition is ready
and would couple presentation refresh to process timing.

### Replace the session with a generic event bus

Rejected because it would make the semantic repository state less explicit,
not more. The session already provides the correct locality for this value.

## Consequences

- New adapters can consume one coherent repository snapshot.
- `GitPanel` no longer wires four correlated state signals.
- Busy, availability, branch, and entries are testable together.
- The repository state has one public publication seam, reducing ordering
  assumptions for future adapters.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testGitRepositorySessionSequencesStatusAndRemote -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testGitRepositorySessionTreatsEmptyRootAsInformational -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
