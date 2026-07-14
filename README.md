# Helios

<div align="center">
  <img src="editor/helios-icon.svg" width="96" alt="Helios icon">

  <p><strong>A desktop IDE for Zith projects, built with Qt 6.</strong></p>

  <p>
    <img alt="Build" src="https://github.com/GalaxyHaze/Helios-IDE/actions/workflows/build-artifact.yml/badge.svg">
    <img alt="Release" src="https://img.shields.io/github/v/release/GalaxyHaze/Helios-IDE">
    <img alt="Qt 6" src="https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white">
    <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white">
    <img alt="CMake" src="https://img.shields.io/badge/CMake-3.21%2B-064F8C?logo=cmake&logoColor=white">
  </p>
</div>

Helios is a native editor focused on the Zith ecosystem. It currently ships with a code editor, LSP integration, workspace search, file tree navigation, a basic Git panel, diagnostics, snippets, context switching, and persistent editor preferences.

## Navigate

- [Overview](#overview)
- [Features](#features)
- [Project Layout](#project-layout)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [Zith Integration Notes](#zith-integration-notes)
- [Build Artifacts](#build-artifacts)
- [Roadmap](#roadmap)
- [Current Caveats](#current-caveats)

## Overview

Helios is not a generic text editor wrapper. The current app is shaped around a Zith-first workflow:

- multi-tab code editing with syntax highlighting
- LSP-backed completion, hover, diagnostics, definition and signature help
- workspace explorer and search
- Git status, staging, unstaging and commit flow
- project context switching
- persistent editor preferences such as font size and word wrap

The UI is built in Qt Widgets, so it stays fast, native and easy to iterate on without pulling in a browser runtime.

## Features

### Editing

- multi-tab editor with line numbers
- syntax highlighting for Zith source
- snippet expansion
- auto-close pairs, indentation helpers and find/replace
- breadcrumb path display and editor status information

### Language Tooling

- Zith LSP client process management
- incremental `textDocument/didChange` sync with debounce
- diagnostics panel
- hover, completion, definition and signature help wiring

### Workspace

- file tree explorer
- workspace search panel
- basic project bootstrap for new Zith projects
- context save/restore across opened files

### Git

- repository detection
- branch/status overview
- `stage all`
- `stage selected`
- `unstage selected`
- commit with message

## Project Layout

```text
.
|-- editor/                 # Core UI, editor widgets, panels and services
|-- docs/                   # Project notes and roadmap
|-- main.cpp                # Qt application bootstrap
|-- CMakeLists.txt          # Build definition
`-- .github/workflows/      # CI and release automation
```

Key modules inside `editor/`:

- `MainWindow.*`: main shell, layout, commands and wiring
- `Code.*`: editor widget, input behavior and LSP document sync
- `LspClient.*`: JSON-RPC client for `zith-lsp`
- `FileTreePanel.*`, `SearchPanel.*`, `GitPanel.*`, `SettingsPanel.*`: sidebar panels
- `Syntax.*`: highlighting rules

## Requirements

- CMake `3.21+`
- Qt `6` with `Core`, `Gui` and `Widgets`
- a C++17 compiler
- Ninja recommended

For the full Zith-oriented workflow, Helios also expects:

- `zithc` available in `PATH` for compiler actions
- network access on first run so the managed Zith runtime can be cached locally

## Quick Start

```bash
git clone https://github.com/GalaxyHaze/Helios-IDE.git
cd Helios-IDE
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build
./build/Helios
```

If `CMAKE_BUILD_TYPE` is not specified, the project currently defaults single-config generators such as Ninja to `Release`.

## Zith Integration Notes

Helios now resolves the Zith runtime dynamically instead of depending on local development paths.

- on startup it can query the latest `Zith-Lang` release
- it caches the matching `zith-lsp` binary and stdlib under the app data directory
- if a cached runtime already exists, Helios can start from cache first and refresh in the background
- editor snippets are bundled with the application resources

For local development or pinning a custom runtime, you can override the managed bootstrap with:

- `HELIOS_ZITH_LSP_PATH`
- `HELIOS_ZITH_STDLIB_PATH`

## Build Artifacts

This repository includes a GitHub Actions workflow at `.github/workflows/build-artifact.yml` modeled after the `Zith` release flow, but adapted to Helios.

Current artifact behavior:

- builds `Release` binaries for Linux, Linux arm64, macOS, Windows and Windows arm64
- uploads CI artifacts for each platform
- on tagged releases, also attaches the archived build output to the GitHub release
- packages the built executable and a short note about Qt runtime expectations
- deploys the Qt runtime into the Windows release archives so Scoop can install a runnable app

## Install

### Scoop

The repository ships a Scoop manifest at `.github/scoop/bucket/helios.json`.

Today you can install directly from the manifest URL:

```powershell
scoop install https://raw.githubusercontent.com/GalaxyHaze/Helios-IDE/main/.github/scoop/bucket/helios.json
```

### Installer Scripts

Unix-like systems:

```bash
curl -fsSL https://raw.githubusercontent.com/GalaxyHaze/Helios-IDE/main/scripts/install.sh | bash
```

Windows PowerShell:

```powershell
irm https://raw.githubusercontent.com/GalaxyHaze/Helios-IDE/main/scripts/install.ps1 | iex
```

### Homebrew

The Homebrew tap is published at `GalaxyHaze/homebrew-helios`.

Current install flow:

```bash
brew tap GalaxyHaze/helios
brew install --HEAD helios
```

Once the stable release automation has populated the versioned formula, you can use:

```bash
brew install helios
```

## Release Automation

The repository now separates release automation into:

- `.github/workflows/create-release.yml` for tag + release page + artifact build + package manager updates
- `.github/workflows/build-artifact.yml` for cross-platform release builds
- `.github/workflows/package-managers.yml` for Scoop and Homebrew package manager updates

This is intentionally conservative: the workflow does not yet attempt full self-contained Qt deployment on every platform.

## Roadmap

The current roadmap lives in [docs/ide-roadmap.md](docs/ide-roadmap.md).

Current direction:

- stabilize editor and startup plumbing
- remove hardcoded local paths for LSP/snippets/examples
- improve async search and navigation
- deepen Git integration
- keep refining the visual consistency of the activity/sidebar model

## Current Caveats

- First-run LSP bootstrap depends on being able to reach the latest Zith release unless a cached runtime or explicit override is already available.
- Linux and macOS release artifacts are still build outputs rather than polished native installers.
- There is no dedicated automated test suite yet.
- The app is under active iteration and should be treated as a fast-moving tool rather than a frozen product.
