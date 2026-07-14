# Repository Guidelines

## Project Structure & Module Organization
`main.cpp` is the Qt application entry point. Core editor UI and behavior live under `editor/`, with paired `.h` and `.cpp` files for widgets and services such as `MainWindow`, `LspClient`, `DiagnosticsPanel`, and `SnippetManager`. Static assets are bundled through [`editor/resources.qrc`](/home/diogo/Helios/editor/resources.qrc) and include [`editor/helios-icon.svg`](/home/diogo/Helios/editor/helios-icon.svg). Build artifacts should stay in `build/`; do not commit generated files from that directory.

## Build, Test, and Development Commands
This project uses CMake and Qt 6.

- `cmake -S . -B build` configures the project and finds Qt6.
- `cmake --build build` builds the `Helios` executable.
- `./build/Helios` runs the editor locally after a successful build.
- `cmake --build build --target clean` removes compiled outputs for a fresh rebuild.

If CMake fails to configure, verify that `Qt6::Core`, `Qt6::Gui`, and `Qt6::Widgets` are installed and visible to CMake.

## Coding Style & Naming Conventions
Follow the existing C++ style: 4-space indentation, opening braces on the next line for classes and functions, and one class per header/source pair where practical. Use `PascalCase` for classes (`MainWindow`, `FileTreePanel`), `camelCase` for functions (`openFilePath`, `saveContextState`), and the existing `m_` prefix for member fields. Prefer Qt types and signals/slots patterns already used in `editor/`.

## Testing Guidelines
There is no dedicated automated test suite yet. At minimum, contributors should build the project and launch `./build/Helios` to catch integration or UI regressions. When adding logic-heavy code, include a small reproducible manual test note in the PR description describing the file opened, action taken, and expected result.

## Commit & Pull Request Guidelines
Recent history uses short, direct commit subjects such as `breaks up main monolith` and `Small updates`. Keep commit titles brief, imperative, and focused on one change. Pull requests should explain the user-visible impact, note any Qt or platform assumptions, and include screenshots or short recordings for UI changes. Link related issues when applicable and mention the exact build command used for verification.
