---
status: accepted
---

# Incremental architecture evolution

Helios will improve its architecture through behavior-preserving seams and deepened modules rather than a rewrite. New seams should be introduced where a real variation or testing need already exists, and each refactor should leave the existing executable behavior intact while moving policy away from visual coordinators.

## Considered options

- Rewrite the IDE around a new architecture: rejected because it would discard working behavior and make regressions harder to localize.
- Add generic interfaces everywhere before a second adapter or concrete variation exists: rejected because it creates shallow abstractions and increases cognitive load without leverage.

## Consequences

- `MainWindow` can be reduced incrementally by moving one policy cluster at a time.
- Tests should cross the new module interface rather than reach into its implementation.
- Existing behavior, especially LSP synchronization, workspace edits, runtime fallback, and context restoration, is the compatibility contract for future refactors.
- A future ADR may supersede this strategy only if the cost of incremental seams becomes demonstrably higher than a controlled rewrite.
