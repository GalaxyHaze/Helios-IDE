# ADR-0114: Publish a context application event

- Status: Accepted
- Date: 2026-09-27

## Context

`ContextWorkspaceController` coordinates the ordered effects of a context
change: applying the workspace root, updating the context indicator, preparing
the Zith runtime, and restoring persisted editor documents when appropriate.
It then needs to notify downstream policies that the active workspace is
stable.

The former interface accepted an `updateClangdLifecycle` callback. That made
the context module know about one particular language-service policy and
forced future consumers to add more policy-shaped callbacks.

## Decision

Publish `workspaceContextApplied()` after the context projection has completed.
The composition root subscribes to this semantic event and decides to
reconcile clangd. The context controller retains only callbacks for effects
that are part of applying a context itself.

The event is emitted after session restoration, so subscribers observe the
resulting workspace topology rather than an intermediate state.

## Alternatives considered

### Keep the clangd callback

Rejected because the interface encodes a consumer policy instead of the
context lifecycle event.

### Emit directly from `ContextManager`

Rejected because `ContextManager` knows which context changed but not when the
workspace root, session, and runtime projection have finished.

### Reconcile from every context consumer

Rejected because it duplicates ordering knowledge and allows consumers to
observe or act on partially applied context state.

## Consequences

- `ContextWorkspaceController` is independent of clangd policy.
- Downstream policies receive one notification after context application.
- `MainWindow` remains the composition root for language-service reconciliation.
- Tests can observe the semantic event without providing a fake clangd policy.

## Verification

- `cmake --build build -j 2`
- `QT_QPA_PLATFORM=offscreen ./build/test_helios testContextWorkspaceControllerPreservesTransitionSemantics -platform offscreen`
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`
- `git diff --check`
