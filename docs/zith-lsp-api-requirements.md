# Zith LSP Server API Requirements for Helios Editor

To unlock and support the modernized IDE capabilities of Helios, the Zith Language Server Protocol (LSP) server must implement and advertise specific capabilities during the `initialize` exchange.

Below is the contract detailing the capabilities, methods, and expected behaviors for declaration, references, rename, code actions, and document symbols.

## 1. Summary of Capabilities and UI Controls

| Feature | LSP Capability | Method Name | UI Impact |
| :--- | :--- | :--- | :--- |
| **Go to Definition** | `definitionProvider` | `textDocument/definition` | Enabled in editor context menu & via Ctrl+Click/F12. |
| **Go to Implementation** | `implementationProvider` | `textDocument/implementation` | Enabled in editor context menu. |
| **Go to Declaration** | `declarationProvider` | `textDocument/declaration` | Enabled in editor context menu (no longer a stub). |
| **Find Usages** | `referencesProvider` | `textDocument/references` | Enabled in editor context menu (no longer a stub). |
| **Rename Symbol** | `renameProvider` | `textDocument/rename` | Enabled in editor context menu (no longer a stub). |
| **Extract Method** | `codeActionProvider` | `textDocument/codeAction` | Enabled in editor context menu (no longer a stub). |
| **Document Symbols / Outline** | `documentSymbolProvider` | `textDocument/documentSymbol` | Populates the collapsible right-side Structure/Outline view. |

If any capability is missing or null in the `initialize` response, Helios will display the corresponding action as a **disabled stub** with an explicit tooltip explaining that the capability is not supported by the active LSP server.

---

## 2. Detailed Technical Specifications

### 2.1 Go to Declaration (`declarationProvider`)
Allows navigating to a symbol's declaration.
- **Request Method**: `textDocument/declaration`
- **Params**: `TextDocumentPositionParams`
- **Response**: `Location | Location[] | LocationLink[] | null`
- **Error Behavior**: If unresolved, return `null` or an empty list.

### 2.2 Find Usages (`referencesProvider`)
Finds all references to a symbol at the cursor location.
- **Request Method**: `textDocument/references`
- **Params**:
  ```json
  {
    "textDocument": { "uri": "file://..." },
    "position": { "line": 5, "character": 12 },
    "context": { "includeDeclaration": true }
  }
  ```
- **Response**: `Location[] | null`

### 2.3 Rename Symbol (`renameProvider`)
Renames a symbol throughout the workspace.
- **Request Method**: `textDocument/rename`
- **Params**:
  ```json
  {
    "textDocument": { "uri": "file://..." },
    "position": { "line": 5, "character": 12 },
    "newName": "myNewSymbolName"
  }
  ```
- **Response**: `WorkspaceEdit | null` (consisting of changes mapped by URI).
- **Optionally Supporting**: `prepareProvider` (for `textDocument/prepareRename`) to validate the symbol prior to renaming.

### 2.4 Extract Method / Code Action (`codeActionProvider`)
Performs refactorings, such as extracting the selected code block into a new method.
- **Request Method**: `textDocument/codeAction`
- **Params**:
  ```json
  {
    "textDocument": { "uri": "file://..." },
    "range": {
      "start": { "line": 10, "character": 0 },
      "end": { "line": 15, "character": 20 }
    },
    "context": {
      "diagnostics": [],
      "only": ["refactor.extract"]
    }
  }
  ```
- **Response**: `(Command | CodeAction)[] | null`

### 2.5 Document Symbols / Outline (`documentSymbolProvider`)
Lists hierarchical symbols in the active file to populate the right-side Structure panel.
- **Request Method**: `textDocument/documentSymbol`
- **Params**: `DocumentSymbolParams`
- **Response**: Hierarchical list `DocumentSymbol[]` is highly preferred over flat `SymbolInformation[]`.
  - **Preferred Format**:
    ```json
    {
      "name": "MyFunction",
      "kind": 12,
      "range": { ... },
      "selectionRange": { ... },
      "children": [ ... ]
    }
    ```

## 3. Transport and lifecycle

The server is a stdin/stdout JSON-RPC process using `Content-Length` framing.
Clients must tolerate fragmented frames and extra headers. Helios starts the
process asynchronously, sends `initialize` only after it starts, and sends
`shutdown`, waits for its response, then sends `exit` during an orderly stop.

## 4. Document synchronization and stale responses

`didOpen`, `didChange`, and requests carry the current document version.
Positions and ranges use UTF-16 code units. A client must discard a response or
versioned diagnostic that no longer matches the open document. `$/cancelRequest`
is used for superseded interactive requests; cancellation errors `-32800` and
`-32801` are not user-facing errors.

## 5. Supported requests and notifications

Helios negotiates completion (including resolve and snippets), hover,
definition, declaration, implementation, references, document highlight,
document symbols, rename, formatting, folding ranges, code actions, and full
relative semantic tokens. `window/logMessage`, `window/showMessage`,
`publishDiagnostics`, and `zith/requestSaveAll` are handled. The only custom
experimental capability negotiated is `zith.requestSaveAll`.

## 6. Workspace edits and local commands

Helios accepts local `file://` workspace edits in `WorkspaceEdit.changes` only.
It validates all ranges before changing any file. Build, Check, Compile, and
Run remain local editor actions; they do not use `workspace/executeCommand`.

## 11. Planned, not supported

The following are roadmap items, not an API contract: background workspace
indexing, a reusable stdlib cache, semantic-token delta responses, performance
metrics, and a dedicated build executor. Do not negotiate, invoke, or depend
on these features until an implementation and its integration tests add them
to this guide.
