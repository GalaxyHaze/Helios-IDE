# Changelog - Optimizations and Bugfixes

This changelog records the recent commits and features focused on performance optimizations and bugfixes for the Helios IDE.

## [Unreleased] - 2026-07-15

### Optimized
- **Syntax Highlighter Trimming**: Exclude leading whitespace characters (spaces and tabs) from regular expression search starts (`SyntaxHighlighter::highlightBlock`). This reduces search spaces and speeds up line-by-line parsing.
  - *Contributor*: Bqr1s (PR #3)
  - *Refinement*: Replaced temporary substring copy with zero-copy `globalMatch(text, pos)` to eliminate memory allocations and Qt deprecation warnings.
- **Safe Long-Line Bypass**: Implemented a protective line-length threshold. Lines exceeding 5000 characters bypass complex keyword/string/operator regular expressions to prevent complete UI freezes on minified/large files. Fast multi-line comment logic is preserved to maintain correct syntax state across lines.
  - *Mitigated from*: PR #5 by Bqr1s (avoiding unsafe `QApplication::processEvents()` reentrancy and crash risks).

### Fixed
- **CJK Breadcrumb Font Size**: Restored consistent font sizes in the Breadcrumbs Bar. The file label no longer inherits an oversized 22px+ size by default. Extra height (7px) is now conditionally applied only when CJK (Chinese, Japanese, Korean) characters are actually present in the file path.
  - *Contributor*: Bqr1s (PR #4)
