# Helios IDE

Helios is a desktop IDE for editing Zith, C-family, and plain-text files. This context names the user-facing concepts that connect workspace state, editor documents, language tooling, search, appearance, and runtime tasks.

## Workspace and Session

**Workspace**:
The project root currently being operated on, together with the files and tools associated with that root. A workspace is identified by a normalized filesystem path.

**Workspace root activation**:
The validated transition that makes a directory the root of the current
workspace or creates a new context for it. Activation saves the previous
context before mutation and records the selected root as a recent project.
Choosing a directory is an interaction concern; activation is a workspace
state transition.

**Workspace root selection**:
The user interaction that requests a directory for a workspace-root intent.
Selection owns dialog title and initial-directory policy, but does not
validate, mutate context history, save session state, or persist recents.
Cancellation is represented by an empty selection and produces no activation
effects.

**Context**:
A navigable workspace entry that remembers its root, open file paths, and active tab. Helios can retain multiple contexts and move between them.

**Editor session**:
The materialized set of editor tabs for the active context, including the active tab and the live document state held by those editors.

**Editor document-set change**:
The semantic event that an editor document was opened, received a persistent
path, or was released. It describes a change in session topology or document
identity; it does not mean that text content changed or that a language
service must be reconciled in one specific way.

**Context state**:
The restorable data for an editor session: workspace root, open file paths,
and the active persisted-file index. Untitled tabs are not persisted, so the
index is relative to `openFiles` rather than to the live tab widget. It does
not include the full widget state of each editor. The active context is
captured before application shutdown so document changes made since the last
workspace transition are not lost.

**Context change reason**:
The domain meaning of a context notification: navigation to an existing
context, replacement of the active context's workspace root, or creation of a
new context. Consumers use this meaning to decide whether session state should
be restored; it is distinct from the context's stored root and session data.

**Workspace context application**:
The completed projection of a context change into the active workspace:
workspace root, context indicator, session restoration, and required runtime
preparation have been applied. It is a semantic notification for downstream
policies; it does not name a specific language service or presentation action.

**Workspace command state**:
The coherent facts needed to decide whether workspace commands can be
executed: language-service command capability, active Zith editor, current
file, workspace root, running task, and formatting support. It is distinct
from the enabled/tooltip presentation projection shown by the command surface.
The root value in this state is also the root used to construct a build or run
request.

## Documents and Language Services

**Document**:
An editable file identified by a path and URI, with text, a local version, modified state, and optional diagnostics.

**Language identity**:
The language associated with a document. It determines syntax highlighting, the language identifier sent to LSP, and which language service receives the document.

**Language service**:
The language-aware capabilities available to a document, such as completion, hover, navigation, diagnostics, formatting, symbols, and code actions.

**LSP runtime**:
The executable, standard library, release tag, and origin used to provide a language service.

**LSP start options**:
`LspStartOptions` is the named composition value used to start one language
service. It keeps server path, standard-library path, workspace root, and
initialization mode together so lifecycle coordinators do not pass several
same-typed strings positionally.

**LSP value objects**:
The protocol-facing values shared by transport, editor coordination, and
result decoding: positions, ranges, document changes, diagnostics, locations,
completion items, hover information, signature help, and start options. They
carry data and invariants, not process lifecycle or UI policy.

**LSP initialization profile**:
The protocol-level capabilities, workspace root, and server-specific startup
options negotiated when a language service starts. Zith and clangd share the
common language-service contract but have different experimental capabilities
and initialization options.

**LSP server capabilities**:
The negotiated set of language-service providers and the document
synchronization kind advertised by one server session. It is a protocol value,
not a lifecycle object and not a UI availability policy.

**LSP capability support query**:
The semantic question of whether one negotiated provider is available on the
current language-service session. Callers name a `Capability` rather than
depending on one getter per provider; the client keeps the mapping from that
vocabulary to the protocol value.

**Runtime identity**:
The materialized association between one LSP runtime and a workspace. It
includes the executable, standard library, release tag, and workspace root,
and is distinct from the status text shown while that runtime is resolving,
starting, ready, or failing.

**LSP runtime presentation state**:
The named snapshots used to project language-service state into presentation
surfaces. `LspRuntimeInfo` describes the managed Zith runtime, `ClangdInfo`
describes the C-family service, and `LspDiagnosticsInfo` describes connection
and synchronization diagnostics. These values are presentation inputs, not
the source of runtime lifecycle or persisted settings.

**LSP runtime presentation model**:
The stateful presentation policy that retains the latest runtime, clangd, and
diagnostic snapshots and produces their display values with locale-aware
fallbacks. It is shared by the Settings and LSP Manager adapters; it does not
own widgets, layout, styling, lifecycle, or persistence.

**Language-service workspace configuration**:
The coherent workspace-level conditions under which language services may
participate: whether language services are enabled, whether C-family support
is enabled, which clangd executable is available, and which workspace root is
active. It is distinct from runtime presentation state and from the set of
currently open documents.

**Language-service settings persistence**:
The small persistence interface for the language-service preferences that
survive launches: global enablement, online Zith runtime preference, C-family
enablement, and the clangd path override. Its TOML adapter owns translation to
the live settings store; runtime controllers consume the interface rather than
knowing the file format.

**Document synchronization**:
The lifecycle that keeps a local document and its language service aligned through open, change, save, and close operations with document versions.

**LSP document protocol**:
The language-service protocol role that serializes document open, change, save,
and close notifications while owning the document-version freshness invariant
and invalidating requests that target an older document state. It is distinct
from editor text buffering and from the language-service process lifecycle.

**Editor document synchronization**:
The editor-side responsibility that batches local text changes, advances the
local document version, and chooses the incremental or full representation
needed by the active language service. It is distinct from the LSP document
protocol and from the text widget's input handling. A failed or unavailable
delivery preserves the pending batch for a later flush; it is not equivalent
to synchronization. `markDocumentSynchronized` is the explicit baseline
transition used after a current document has been opened with the language
service, and clears only changes already represented by that open snapshot.

**Editor session controller**:
The module that owns editor-tab creation, persistent file opening, path
assignment, saving, release, and session capture/restore. It publishes an
editor document-set change but does not decide language-service lifecycle
policy or presentation layout. Its editor-preference input is a semantic
value describing font, word-wrap, and Vim enablement; the session applies that
value consistently to new and existing tabs.

**Editor preferences**:
The session-level configuration of editor presentation and interaction
defaults: editor font, word-wrap mode, and Vim enablement. These preferences
are supplied by application settings but their application to materialized
editor tabs belongs to the editor session.

**Editor appearance**:
The visual treatment of an editor surface, including its text area, selection,
current-line emphasis, gutter, bracket matches, and diagnostic emphasis. It is
part of the IDE's visual language and is distinct from document content,
language-service state, and editor input behavior.

**Editor decorations**:
The document- and cursor-derived visual selections projected onto one editor
surface: diagnostics, bracket matches, language-service highlights, find
matches, and current-line emphasis. Decorations are presentation state, not
document content; their composition belongs to `EditorDecorationController`,
while color roles belong to `EditorAppearanceController`.

**Editor context menu**:
The transient action surface for language-aware operations and symbol-local
editing actions at the current editor location. Its availability reflects the
active language service, while the document and editor session remain the
sources of truth for the selected symbol and current document state.

**Document sync batch**:
The versioned set of pending local edits that a document sends to its language
service. The batch preserves ranges relative to the previous snapshot and can
be emitted incrementally or as the complete document when the server only
supports full synchronization.

**LSP request lifecycle**:
The pending work associated with a language-service request, including its
document URI and version, cancellation/replacement policy, timeout, response
completion, and cleanup when the client stops. Request lifecycle is distinct
from document synchronization; the client still rejects responses for stale
document versions.

**LSP request sender**:
The protocol capability that creates and sends a request, assigns its
identifier, tracks cancellation and replacement, and removes pending work
when it completes or fails. It also applies response freshness and dispatches
the completion callback or protocol error through the transport-facing
session. It is distinct from the language-service policy that chooses which
request to make.

**LSP response dispatch**:
The request-lifecycle transition that consumes a response identifier, rejects
work for an obsolete document version, suppresses normal cancellation errors,
and delivers either the response callback or a user-visible protocol failure.

**LSP feature request router**:
The language-service protocol role that maps an editor feature intent to its
method, payload, cancellation policy, and result decoding. Position features
and document features share this protocol role even though their result values
are different; editor controllers remain responsible for deciding when a
feature is available and should be requested.

**LSP session lifecycle**:
The process-session state machine that starts a configured language service,
negotiates initialization, defines readiness, performs the shutdown handshake,
escalates an unresponsive shutdown, and carries a pending replacement start.
It is distinct from transport framing, request tracking, document
synchronization, and editor feature selection.

**Diagnostics**:
Problems or informational messages associated with a document URI, source range, severity, and document version.

**Workspace edit**:
A multi-document edit requested by a language service. Helios currently accepts local-file text changes, validates their ranges before mutation, and applies them to open documents or saved files.

## Search and Editing

**Search result**:
An occurrence found in a workspace file, including its path, line, column, and preview text.

**Workspace search session**:
The asynchronous scan of a workspace for a query or replacement preview. A
session owns its scan policy, root path, result limit, and cancellation token;
results from an older session must not reach the search surface. It is
distinct from the search panel's debounce, result rendering, and replacement
application.

**Workspace search policy provider**:
The composition seam that supplies the current search extensions and excluded
directories when a workspace search starts. The provider belongs to the host
that owns settings; `SearchPanel` only forwards it to
`WorkspaceSearchController` and does not read global preferences directly.

**Replace plan**:
The validated set of range-and-text edits that describes a replacement before it is applied to documents or files.

**Open-document replace**:
A replacement applied to a live editor document, leaving the document modified until the user saves it.

**Vim mode**:
The current interaction mode of the editor's Vim behavior, such as Normal or Insert.

**Vim search session**:
The transient interaction that collects a forward or backward search query,
confirms or resets that query, remembers the last completed search, and
repeats it from the current document position. It is distinct from workspace
search results and from Vim motion interpretation.

**Vim motion**:
A repeatable cursor movement in Vim interaction, represented by a document
movement and applied with an effective count. Its application remains
separate from command state, selection state, and editor presentation.

**Vim motion resolver**:
The pure policy that maps the shared repeatable Vim motion keys to named
movement values. It is distinct from the controller that applies movement and
handles commands with additional state, such as `g`, `^`, `$`, and visual
selection.

**Vim character search**:
The line-local Vim search that moves to a repeated character occurrence with
`f`, `F`, `t`, or `T`, and remembers the last successful search for `;` and
`,` repetition. It is distinct from multiline textual search.

**Vim pending operation session**:
The transient state machine for an operator such as delete, change, or yank
waiting for its motion or line command. It owns the pending anchor, count,
selection completion, and cancellation; the global Vim controller only
orchestrates mode changes from its named result.

**Vim command session**:
The transient `:` interaction that collects a command-line string, supports
editing and cancellation, and produces one completed Vim command. It is
distinct from the application action that interprets the submitted command
and from multiline text search.

**Vim command**:
A command emitted by Vim mode for application-level actions such as saving or closing the current document.

**Editor file commands**:
The user-facing intentions to create an untitled document, open a path, or save
the current document. Their dialog and feedback policy is distinct from the
session's file-write and document-synchronization policy.

**Editor file controller**:
The module that translates file commands from the shell, Vim, and tab-close
flow into dialog selection and `EditorSessionController` operations. It owns
Save As interaction and post-save command-availability refresh; it does not own
document persistence or LSP synchronization.

**Workspace root interaction controller**:
The module that translates folder-selection intents into workspace-root
activation. It owns the distinction between replacing the current root and
creating a context, the dialog titles, and the initial-directory policy. It
delegates validation and mutation to `WorkspaceRootController` and does not
mutate `ContextManager` directly.

**Workspace command result**:
The normalized outcome of a language-service workspace command, including
transport success, server success, program URI, task identity, code-generation
availability, and diagnostic text. It is distinct from the compiler panel
presentation and from the task-output lifecycle.

## Appearance and Preferences

**Appearance**:
The effective UI palette, fonts, scale, and rendering strategy applied to the application and editors.

**Theme**:
The named or custom palette and syntax color set used by the application.

**Theme definition**:
The parsed palette, custom color map, and syntax style map that describe a
theme independently of where its JSON was found or how the application
applies it. It is distinct from theme loading policy, fallback selection, and
widget styling.

**Theme definition parser**:
The module that translates the theme JSON schema into a validated theme
definition, including palette defaults and valid custom and syntax colors. It
does not discover files, retain the current theme, choose fallback timing, or
apply styles to widgets.

**Locale**:
The language used for translated UI strings.

**Preference**:
A user-configurable value persisted across launches, including appearance, editor behavior, search filters, workspace layout, and language-service settings.

**TOML settings snapshot**:
The value object containing the complete persisted settings state. It is the
boundary between the live settings store and the persistence codec; it carries
values and defaults but does not perform file I/O, emit signals, or apply
settings to widgets.

**TOML settings codec**:
The pure module that parses and serializes the settings TOML representation.
It owns format details, list encoding, legacy `fontFamily`/`fontSize`
migration, and value normalization. It does not own the settings file path,
auto-save, live mutation, or preference notifications.

**TOML settings store**:
The application-owned live settings module. It owns the current preference
state, configuration path, load/save lifecycle, public setters, compatibility
aliases, and `editorPreferencesChanged`; it delegates format interpretation to
the TOML settings codec.

**Shortcut catalog**:
The shared application vocabulary of shortcut translation keys and displayed
key sequences. It is the source for shortcut surfaces such as Settings and the
Shortcuts dialog; those widgets render the catalog but do not redefine it.

**Shortcut presentation**:
The translated tree projection of the shortcut catalog, including category
grouping, stable shortcut metadata, displayed key sequences, and expanded
categories. It is shared by shortcut surfaces, while each surface retains
ownership of its own layout, sizing, and theme styling.

## Runtime Tasks

**Runtime resolution**:
The process of selecting a usable LSP runtime from environment overrides, local development files, installed cache, or a remote release.

**Runtime asset download**:
The transport operation that fetches one release asset into a temporary cache
path. It owns request validation, network timeout, cancellation, and atomic
file materialization; it does not decide which runtime source wins or install
the asset into a release.

**Runtime toolchain orchestration**:
The policy that chooses between environment overrides, local development
runtime, installed cache, and remote release, then coordinates asset downloads
and installation. It owns fallback and readiness decisions, while transport
details belong to the runtime asset download module.

**Zith release catalog**:
The module that interprets remote Zith release metadata and selects the
platform-compatible LSP and standard-library assets. It is pure with respect
to network and cache I/O; release fetching belongs to runtime orchestration,
and installed-runtime discovery belongs to the runtime catalog.

**Zith runtime catalog**:
The module that describes and validates installed Zith runtimes in the local
cache, including cache paths, release tags, executable/stdlib contents, newest
release selection, and stale local-cache removal. It does not fetch releases
or own language-service process lifecycle.

**Zith runtime installer**:
The module that turns one downloaded runtime asset into an installed cache
entry, including directory creation, executable permissions, and
platform-specific archive extraction. It does not choose releases, fetch
network payloads, or decide fallback policy.

**Runtime task**:
A build, check, run, or stop operation requested from the language service and represented in the compiler panel.

**Workspace command orchestration**:
The policy that starts, tracks, reports, and stops runtime tasks for the active
workspace. It includes the association of process output and exits with task
identifiers.

**Workspace command execution capability**:
The executor's decision that the active LSP policy, client readiness, and
`workspace/executeCommand` capability permit a workspace command request. It is
shared by command execution and shell-action availability; it is not the same
as having an active workspace root or a suitable editor for a particular
command.

**Workspace command availability**:
The derived state that says whether build, check, format, run, and stop intents
are currently actionable, together with the explanation shown when an intent
is unavailable. It is distinct from command execution and from the menu that
presents those intents.

**Workspace command availability controller**:
The module that derives workspace command availability from the active editor,
language-service capabilities, and task state, then publishes the value to the
shell command surface. It does not start or stop tasks.

Workspace command availability uses the executor's semantic precondition
decision for Build, Check, Run, and Stop; it owns the availability value and
tooltips, not a second execution policy.

**Workspace replacement**:
The confirmed mutation of search targets across the workspace. It applies
edits to open editors in memory and to closed files through safe writes, then
reports the aggregate result and refreshes search presentation. It is
distinct from scanning and from the search panel's input and preview.

**Workspace replace controller**:
The module that owns replacement confirmation, target application, error
reporting, and search refresh. The host supplies adapters for open editors,
confirmation, status messages, and search refresh; it does not duplicate the
mutation policy.

**Workspace search controller**:
The module that owns asynchronous workspace scanning and replacement-preview
production. It samples the active scan policy when a session starts, cancels
older sessions when a root or query changes, and emits only results from the
current session. It does not render widgets or apply replacement edits.

**Search panel**:
The Qt adapter for search input, debounce, result rendering, and file
activation. Its scan policy is supplied through a provider at composition time;
the panel does not own settings persistence or decide which extensions are
searchable.

**Git status snapshot**:
The value representation of `git status --short --branch` used by the source
control surface. It contains the branch label and changed relative paths with
their two-column Git status, but no widgets, process state, or theme choices.

**Git status presentation**:
The projection of the status entries in the latest `GitRepositoryState` into
the source-control list. A snapshot replaces the visible rows as a whole; it
is not an append-only event stream. Theme changes re-render the stored
repository state without starting a new Git operation, and selection is
identified by relative path.

**Git status parser**:
`GitStatusParser` converts Git's porcelain status output into a
`GitStatusSnapshot`. `GitCommandRunner` owns process execution;
`GitRepositorySession` owns the source-control operation workflow; `GitPanel`
owns row rendering and user interaction; the parser owns only text
interpretation, including rename target normalization.

**Git command execution**:
The asynchronous operation that runs one Git command in a workspace, captures
standard output and error, and classifies success, start failure, or timeout.
The execution module owns process lifecycle and timeout behavior; operation
meaning and sequencing belong to the repository session.

**Git repository session**:
The workspace-scoped source-control state and operation workflow. It accepts
repository intents such as refresh, stage, unstage, commit, initialize, and
connect remote; it publishes status, remote availability, busy state, and
user-facing operation messages without knowing widgets or theme choices.

**Git repository state**:
The coherent snapshot published by `GitRepositorySession`: root path, branch,
status entries, repository availability, remote availability, and busy state.
Adapters consume this value as one transition rather than reconstructing state
from independent notifications. The session remains the source of truth for
process sequencing and refresh policy.

**Git panel presentation state**:
The projection of a `GitRepositoryState` into source-control controls: branch
label, summary message, initialization/remote actions, status-list visibility,
and busy state. `GitPanelPresentationModel` computes this value without
widgets or Git I/O; `GitPanel` applies it to Qt controls and delegates row
projection to `GitStatusListPresenter`.

**Progress token**:
The identifier used to associate language-service progress notifications with a runtime task or other long-running operation.

**LSP log projection**:
The presentation of language-service messages into shell panels. The settings
panel receives messages only while the language service is enabled; the LSP
manager receives them regardless so it can explain disabled or startup
conditions. This is a presentation policy, distinct from message production
and runtime lifecycle.

**LSP runtime presentation controller**:
The module that derives named runtime, clangd, and diagnostics snapshots from
language-service lifecycle state and projects them to the settings panel and
LSP manager dialog. It does not own either panel's layout, fallback text, or
runtime lifecycle; locale-aware fallback projection belongs to the LSP runtime
presentation model.

## Architecture Boundaries

**Language identity**:
`LanguageIdentity` classifies paths and provides stable LSP identifiers. It is
the source of truth for routing and syntax selection.

**Language-service workspace routing**:
The active binding of document languages to language-service clients, including
settings-derived enablement and the set of open C-family documents that drives
clangd lifecycle reconciliation. It is distinct from protocol transport and
from presentation of service status.

**Language-service workspace controller**:
The module that updates document-service bindings, resolves clients by path,
recognizes language-specific editors, and reconciles clangd from workspace
state. The host supplies settings, executable discovery, workspace root, and
presentation callbacks.

**Document synchronization**:
`LspDocumentCoordinator` binds editors to language clients and owns open,
change, save, and close transitions. It must not own widgets or settings.
Opening a document sends the editor's current text and then marks the editor
sync controller's baseline; this prevents changes made before client readiness
from being replayed after the full snapshot. Closing or detaching is the
explicit discard boundary for pending delivery state.

**Editor diagnostic highlighting**:
`EditorDiagnosticHighlighter` projects LSP diagnostic ranges into Qt text
selections and applies severity colors. It clamps protocol positions to the
available document rather than allowing malformed server data to escape into
the editor widget; `CodeEditor` retains ownership of the document and the
resulting selections.

**Editor decoration controller**:
`EditorDecorationController` owns the composition of editor-local
`QTextEdit::ExtraSelection` sources. It projects diagnostics, brackets, LSP
highlights, find selections, and the current-line marker through one widget
update. It consumes appearance roles but does not resolve themes, own document
state, or handle language-service requests.

**Editor language feature controller**:
`EditorLanguageFeatureController` owns language-service interactions bound to
one editor: completion, signature help, navigation, references, formatting,
rename and code-action requests. Its feature-availability interface combines
client readiness with negotiated provider capabilities, so menu and keyboard
input use the same preconditions. `CodeEditor` owns input and menu
presentation but delegates feature requests through this seam instead of
inspecting protocol capability details.

**Editor language feedback presenter**:
The editor-local presentation of accepted language-service results: diagnostics,
hover and signature help tooltips, navigation locations, and document
highlights. It attaches to one language client and filters by document URI and
version before asking `CodeEditor` to render or emit a navigation intent. It
does not initiate requests or own document synchronization.

**Git status list presenter**:
`GitStatusListPresenter` projects one `GitRepositoryState` snapshot into the
source-control file list. It owns row construction, status-badge styling,
relative-path selection preservation, and extraction of selected paths for
stage/unstage commands. `GitPanel` retains command intent, repository
feedback, branch/summary presentation, and file activation; the presenter
does not run Git or open files.

**Git panel presentation model**:
The pure policy that maps coherent repository state to the controls shown by
the source-control panel. It centralizes the distinction between no repository,
an available repository without a remote, and a clean or changed repository;
it does not render widgets, run Git, or own transient operation messages.

**Editor language request context**:
The document-bound value used to initiate a language-service request. It
contains the document URI, current version, and UTF-16 cursor position, and is
valid only when the editor has an identified document and non-negative
coordinates. The editor creates it after flushing pending changes; request
controllers do not reconstruct these invariants from widget internals.

**Editor text edit application**:
`EditorTextEditApplier` converts LSP positions to document offsets, orders
non-overlapping edits from the end of the document toward the beginning, and
applies them as one edit block. It owns range clamping and edit ordering; the
editor widget retains ownership of the document.

**Editor completion insertion**:
`EditorCompletionInsertionPolicy` decides which current-word interval a
completion replaces and expands the supported LSP snippet placeholders. It
receives line text and UTF-16 cursor position, returns a value decision, and
does not know `QTextDocument`, widgets, or language-service lifecycle.
`CodeEditor` retains ownership of applying the decision to its document.

**LSP document registry**:
The language-client state that records the latest version of each open
document. It is the source of truth for accepting versioned responses and
diagnostics; closing a document removes its version, while notifications
without a version remain admissible.

**LSP request sender module**:
`LspRequestSender` owns JSON-RPC request envelopes, request IDs, cancellable
replacement, timeout cancellation, URI cancellation, and pending-request
cleanup. `LspClient` keeps request-specific callbacks, stale-version checks,
result decoding, and application signals.

**LSP server message dispatcher**:
`LspServerMessageDispatcher` owns messages initiated by the language server:
server requests that need a JSON-RPC reply, diagnostics version filtering,
standard LSP notifications, and Zith runtime notifications. It receives
callbacks for writing replies, process state, and current document versions,
and publishes translated events. It does not own client-initiated requests,
process lifecycle, widgets, or request tracking.

**LSP protocol framing**:
The transport-level conversion between JSON-RPC objects and `Content-Length`
framed byte streams. It includes incremental buffering, message-size limits,
malformed-header recovery, and JSON object extraction, but no process lifecycle,
request policy, document state, or application signals.

**LSP protocol codec**:
`LspProtocolCodec` is the stateful, non-visual module that implements LSP
framing. `LspProcessTransport` owns the `QProcess`, incremental reads, writes,
and conversion between frames and JSON objects; the codec owns only bytes
waiting for a complete frame and reports framing errors through value results.

**LSP process transport**:
The process-facing adapter for one language-service executable. It owns
`QProcess`, incremental protocol decoding, framed writes, stderr chunks,
process errors, and exactly one completion notification per process. Its
completion interface publishes an `LspProcessResult` value rather than
exposing `QProcess` types. It does not know requests, document versions,
capabilities, or UI policy; `LspClient` uses it as the transport seam and owns
protocol/application dispatch.

**LSP process result**:
The transport value describing one process completion: exit code and whether
the process crashed. It is an infrastructure result, not a Qt process-status
type and not a language-service lifecycle decision.

**LSP result decoding**:
The conversion of JSON-RPC result and notification payloads into Helios
language-service values such as ranges, locations, completion items, hover
information, edits, signatures, and diagnostics. It is distinct from framing,
request lifecycle, and signal presentation.

**Editor LSP lifecycle**:
`LspEditorLifecycleController` keeps the collection of visible editor
documents aligned with a particular language client as that client becomes
ready or stops. It protects client ownership across tabs and delegates each
editor transition to the `LspDocumentCoordinator`, which owns language
binding, version, and transport policy. It does not own runtime restart
policy or UI status.

**LSP restart policy**:
The recovery decision for an unexpected Zith language-service stop. It limits
rapid restart loops and chooses bounded backoff; it is separate from document
lifecycle, process scheduling, and status presentation.

**Clangd lifecycle**:
`ClangdLifecycleCoordinator` reconciles clangd process state from explicit
configuration. It owns process policy, not UI wording or document presentation.

**Clangd executable resolution**:
`ClangdExecutableResolver` resolves the executable used by C-family language
services. An explicit configured path takes precedence; otherwise it searches
the supplied `PATH` directories for an executable named `clangd`. The policy
does not read settings or environment variables itself, so composition roots
can provide those inputs and tests can exercise resolution deterministically.

**Zith runtime state**:
The value object that records one materialized Zith runtime identity and its
presentation status. It supports identity comparison and invalidation without
owning process lifecycle, runtime resolution, widgets, or settings.

**Zith runtime lifecycle**:
The policy that resolves, activates, restarts, disables, and invalidates the
Zith language-service runtime for the current workspace. It is separate from
clangd lifecycle and from the presentation of runtime status.

**Zith runtime lifecycle coordinator**:
The module that owns Zith runtime resolution and process transitions. It
publishes state and lifecycle events while leaving widgets, panels, settings,
and document presentation to the application shell.

**Zith runtime override resolution**:
The deterministic policy for interpreting explicit Zith runtime paths supplied
by the environment. Both paths absent means the managed runtime may continue;
a partial override is ignored so fallback remains possible; invalid complete
overrides are terminal configuration errors; and two valid paths form the
runtime identity to use.

**Editor syntax**:
`EditorSyntaxController` owns highlighter creation, replacement, and cleanup.
`MainWindow` supplies the language identity and should not maintain a second
highlighter map.

**Workspace commands**:
`WorkspaceCommandController` owns Zith build, check, run, and stop policy,
including command-result interpretation and the execution-capability decision.
`WorkspaceTaskOutputController` owns early process-output buffering, process
exits, progress, task announcement, and cleanup when the LSP stops. The
command-state snapshot includes the active workspace root and tracked task
identifier so one operation does not mix facts read from different moments.
Build, Check, and Run additionally require an active Zith editor; Stop may
continue without one because a task can outlive the selected document.
`MainWindow` supplies presentation and active-workspace/editor callbacks; it
does not duplicate task tracking, capability evaluation, or result parsing.

**Workspace command event intake**:
The connection between the Zith language client and workspace-command policy.
Command results and save-all requests enter through the command controller;
process output, process exit, progress, and process stops enter through the
task-output collaborator. This keeps each protocol event beside the state it
mutates without putting all event policy in one shallow module. The command
controller publishes task-state changes through one `stateChanged` signal
carrying the complete `WorkspaceCommandState` snapshot; action refresh must
not be wired through a duplicate callback path.

**Workspace task output**:
The document-independent task stream associated with a Zith workspace
command. Output and exits may arrive before the server announces a task
identifier, so they are buffered by task identifier and projected when the
task is announced. The task-output module owns this ordering invariant and
the `CompilerPanel` projection; command eligibility remains a workspace
command concern.

**Editor session**:
`EditorSessionController` owns the lifecycle of materialized editor tabs and
their document-service synchronization, including capture and restoration of
the session state. Restoration preserves the persisted active file when it is
available; missing files are skipped and the first successfully restored file
is selected when the persisted active file cannot be opened. `MainWindow`
supplies window-specific editor setup and presentation callbacks.

**Editor tab-close policy**:
The interaction policy for closing a materialized editor tab. An unmodified
editor can be released immediately; a modified editor requires an explicit
save, discard, or cancel decision, and a save must complete before release.
This is distinct from editor-session materialization and document
synchronization.

**Editor tab-close controller**:
The module that enforces tab-close ordering and decision handling. It observes
tab-close requests, delegates the visual decision and save operation through
callbacks, and releases an editor only when the decision permits it, the save
adapter reports success, and the document is no longer modified. It does not
own dialogs, file I/O, or editor session state.

**Editor interaction**:
The policy that turns editor signals and Vim commands into user actions. It
keeps save/close intent, modified-document refusal, zoom, mode presentation,
and editor-specific navigation separate from tab materialization.

**Editor interaction controller**:
The module that attaches editor interaction signals and interprets the
supported application-level Vim commands. It delegates save, close, status,
and shell updates through callbacks without owning tabs or documents. Save
callbacks report success explicitly; `wq` does not close an editor after a
failed save.

**Vim document operations**:
The document-local mutation vocabulary used by Vim interaction: movement,
line/selection delete, yank, change, paste, replace, and opening lines. It
does not interpret key sequences or own Vim modes.

**Vim document operations adapter**:
`VimDocumentOperations` applies semantic Vim mutations to a
`QPlainTextEdit`; `VimSearchSession` owns the query lifecycle and document
search navigation; `VimMotionResolver` owns the shared pure mapping from
motion keys to document movements; `VimPendingOperationSession` owns the
operator-pending state machine; and `VimMotionController` owns key
interpretation for remaining commands, visual state, and mode transitions.
Neither adapter emits Vim commands or decides application-level file actions.

**Editor language-feature interaction**:
The document-local policy that turns LSP results, timers, keyboard gestures,
mouse gestures, and cursor movement into language-aware editor actions. It
includes completion, signature help, hover, navigation, formatting, and
document highlights while preserving URI and document-version checks.

**Bracket matching**:
The document-local calculation that identifies the bracket under or adjacent
to the cursor and its matching bracket, respecting nested occurrences of the
same bracket kind. It returns positions only; highlighting and colors belong
to editor presentation.

**Bracket matcher**:
`BracketMatcher` is the pure module that implements bracket matching for one
line of text. `CodeEditor` owns cursor access and turns a valid
`BracketMatch` into `QTextEdit::ExtraSelection` values; the matcher does not
know widgets, palettes, or rendering.

**Editor typing policy**:
The document-local decision policy for automatic indentation and paired
characters. It identifies whether a character should be ignored, jump over an
existing closing character, insert a pair, or surround a selection. It returns
decisions; the editor owns document mutation, cursor movement, and language
service side effects.

**Editor deletion policy**:
The document-local decision policy for Backspace. It distinguishes deleting an
auto-closed pair, removing one four-space indentation stop, and default editor
deletion. It receives text context only; `CodeEditor` owns the cursor
mutation and Qt fallback.

**Editor language-feature controller**:
The module that owns the interaction seam for document-local language
features. It attaches and detaches one `LspClient`, consumes the relevant
editor events, and projects accepted results through `CodeEditor`'s existing
private state and public navigation signals. It does not own document text,
file identity, document versioning, diagnostics storage, or document
synchronization.

**Status-bar presentation**:
The persistent presentation of workspace context, language-service state,
diagnostics, cursor position, language, encoding, indentation, and Vim mode.
It is a presentation concern distinct from runtime or document state.

**Status-bar presentation controller**:
The application-shell module that keeps the status-bar presentation coherent
across interaction updates and theme changes. Consumers publish semantic
values rather than manipulating individual labels.

**Onboarding dialog**:
The modal help surface shown when a new installation has not dismissed the
getting-started guidance. It is distinct from the welcome workspace surface:
the former explains the product, while the latter starts or selects work.

**Application style**:
The global visual language applied across shell menus, docks, status surfaces,
inputs, buttons, scrollbars, and focus states. It is derived from semantic
theme values and is distinct from the behavior of any individual panel.

**Application theme projection**:
The materialization of the current semantic theme into the Qt palette and the
application-shell surfaces. It includes global and local styles for the
window, tabs, splitter, breadcrumbs, and status bar, but does not choose the
theme or persist appearance preferences.

**Application theme controller**:
The module that applies one coherent theme projection to the shell surfaces.
It consumes `ThemeManager` tokens, `ApplicationStyle`, and effective font
settings, while `MainWindow` decides when to reapply it. Visual changes must
also update the corresponding `.md` and `.html` design pair under
`docs/design/`.

**Shell command surface**:
The visible command vocabulary of the application shell: file, workspace,
tooling, view, and help commands. It is distinct from the policies that
execute those commands and from the document/runtime state used to enable
them.

**Shell translation projection**:
The materialization of the active locale into shell menus and activity-bar
tooltips, including the shortcut text shown to users. It is distinct from
loading/persisting the locale and from translations owned internally by
individual panels.

**Shell translation controller**:
The module that applies one translation policy to `ShellCommandSurface` and
`ActivityBar`. It owns shell translation keys and tooltip formatting while
`MainWindow` decides when the locale has changed. If a visual shell aspect is
altered together with its text, the corresponding `docs/design/` `.md` and
`.html` pair must be updated.

**Shell dialog presentation**:
The shell policy for lazily creating, toggling, raising, and refreshing
application dialogs such as Preferences, Shortcuts, Vim Help, and the LSP
Manager. It is distinct from the runtime and language-service presentation
projected into the LSP Manager.

**Shell dialog controller**:
The module that owns dialog presentation lifecycle, recognizes dialog-related
shell intents, and exposes the LSP Manager as a presentation destination for
runtime controllers. It does not own LSP state, settings persistence, or
dialog-internal content.

**Shell command intent**:
A user request emitted by the command surface, such as opening a file,
building, formatting, changing a panel, or opening help. The intent vocabulary
is independent from the menu/shortcut surface; the shell interprets the intent
and coordinates the appropriate domain modules.

**Shell command dispatch**:
The domain-specific interpretation of a shell command intent. Workspace task
commands belong to workspace command execution; find/replace belongs to editor
workspace presentation; panel visibility belongs to workspace panel
presentation; and sidebar modes belong to sidebar presentation. The application
shell retains only commands that compose application lifecycle, dialogs,
documents, or language-service policies.

**Domain command handler**:
A module that recognizes and executes only the subset of `ShellCommand` values
for its own policy, returning whether it handled the intent. It must not become
a generic command registry or take ownership of unrelated shell state.

**Context navigation**:
The history-like movement between workspace contexts. Moving through existing
contexts preserves the current context state first; moving right from the end
may create a new context after the user selects a workspace root. It is
separate from context storage and from the restoration of editor sessions.

**Context navigation controller**:
The shell policy that turns context navigation shortcuts into history movement
or a request for new-context creation. It asks the host to persist the active
context and to obtain a workspace root, while workspace root activation owns
validation and context mutation. `ContextManager` remains the owner of
context storage and change notifications.

**Workspace root activation policy**:
The policy that validates and normalizes a selected directory, persists the
active context before changing it, mutates the current context or creates a
new one, and updates recent-project presentation. It is distinct from the
dialogs and panels that collect or display workspace paths.

**Workspace root restoration**:
The read-side transition that validates and applies a persisted workspace root
to the current context without saving the active context or reordering recent
projects. Restoration shares root validation with user-initiated activation,
but it must not turn startup state loading into a new activation side effect.

**Context workspace transition**:
The coordinated transition of the active context into workspace surfaces and
editor session state. A root replacement updates workspace roots without
restoring tabs, but revalidates active language services because their runtime
identity is workspace-root bound; navigation or new-context creation restores
the stored session and may revalidate active language-service runtimes.

**Context workspace controller**:
The module that applies context transitions across workspace-root surfaces,
context indicators, editor-session restoration, and runtime lifecycle hooks.
It receives the semantic change reason from `ContextManager` and uses adapters
for the host's concrete panels and coordinators.

**Workspace navigation**:
The application-shell intake of workspace-originated intentions: opening a file
in the current editor, opening one in a new tab, selecting a workspace root,
requesting a folder, and starting a new project. File tree, welcome, and Git
surfaces emit these intentions; they do not own editor-session or context
transition policy.

**Workspace navigation controller**:
The module that adapts file-tree, welcome, and Git signals into workspace
navigation callbacks, including the shell's New Project and Open Folder
intents. It centralizes the source-to-intention mapping while the host supplies
file opening, root selection, and dialog actions. It does not validate
workspace roots, persist recents, or own editor sessions.

**Sidebar presentation**:
The application-shell state that selects one activity panel, toggles the
sidebar, refreshes Git when its mode becomes active, and keeps the activity
bar synchronized with persisted visibility. It is distinct from the work
performed by the individual panels.

**Sidebar controller**:
The module that owns sidebar presentation policy. It coordinates the activity
bar and panel stack through semantic mode and visibility operations, while the
host supplies persistence and settings-preparation callbacks.

**Workspace panel presentation**:
The shell state of the optional Outline and Bottom panels. It includes their
visibility transitions, persistence of the Outline preference, cleanup or
refresh behavior when the Outline changes, and synchronization of checkable
shell commands. It is distinct from the content and task state owned by those
panels.

**Workspace panel presentation controller**:
The module that keeps Outline and Bottom panel transitions coherent with the
shell command surface. It owns visibility policy and close behavior while the
host supplies the active-editor presentation callback and preference storage.

**Window layout state**:
The application-shell state that survives a launch boundary: main-window
geometry and dock state, sidebar width and visibility, and Outline visibility.
It describes shell arrangement rather than editor-session contents or panel
data.

**Window layout persistence**:
The small persistence interface for reading and writing window layout state.
The TOML settings store is its production adapter; layout policy does not
depend on TOML format or file I/O.

**Window layout controller**:
The module that restores and saves window layout state and translates splitter
movement and panel visibility changes into persistence operations. It owns
startup ordering and the distinction between applying restored state and
persisting a user mutation; it does not own panel content, editor sessions, or
the settings file format.

**LSP runtime presentation**:
The projection of language-service runtime state into the status bar, Settings
panel, and LSP Manager dialog. It includes connection, synchronization,
clangd, and runtime identity text, but not process lifecycle or restart
policy.

**LSP runtime presentation controller**:
The module that keeps the LSP runtime presentation destinations coherent. It
reads state from runtime and lifecycle coordinators, translates semantic
runtime states into status-bar text and theme roles, and publishes named
values and runtime telemetry to the shell's presentation surfaces; the host
decides when those surfaces must be refreshed. It does not own runtime
lifecycle or process policy.

**LSP runtime enablement**:
The application-level transition between an active and inactive language
service runtime. Disabling it stops the C-family client, clears transient
language-service views, updates the runtime presentation, and revalidates
workspace routing; enabling it clears the previous runtime error and requests
the runtime again. This is distinct from document routing and from the
process lifecycle implementation.

**LSP runtime controller**:
The module that owns LSP enablement, runtime refresh, cache clearing, error
state, and the associated action/presentation synchronization. It delegates
preference persistence, clangd reconciliation, confirmation, and status
messaging through explicit adapters. It does not discover language clients or
own the underlying runtime process. It also owns intake of runtime and clangd
configuration events from the Settings panel and LSP Manager dialog, keeping
their transitions consistent while leaving persistence behind adapters.

**LSP runtime event coordination**:
The interpretation of asynchronous runtime and language-server events after
the clients exist. Connected, stopped, failed, initialized, and server-error
events update document attachment, lifecycle state, runtime presentation,
status messaging, logs, and shell availability. This is distinct from the
enablement transition that starts or stops a runtime.

**LSP runtime event controller**:
The module that subscribes to runtime signals and shell-facing language-service
event sources and applies the event-coordination policy. It keeps signal wiring
after all dependent modules are constructed, so event handlers cannot
accidentally target a not-yet-built workspace command module.

**LSP shell event source**:
The small event interface shared by shell consumers that need diagnostics,
messages, lifecycle notifications, workspace-command results, or task output.
`LspClientEventSource` adapts the concrete `LspClient` to this interface, while
tests may provide a fake source. It does not expose client-initiated requests,
feature-specific result routing, document synchronization, or process control.
Those remain responsibilities of the language-service client and their
specialized adapters.

**Language-service feedback**:
The application feedback projected from language-service event sources into the
Diagnostics panel and status bar, including diagnostics, counts, server
messages, and workspace edits returned by rename. It is distinct from
language-service process lifecycle, document synchronization, and
capability-specific result routing.

**Language-service feedback controller**:
The module that applies one feedback policy to every attached LSP shell event
source.
It routes diagnostics to the shared panel, publishes diagnostic counts, turns
server messages into status presentation, and delegates rename workspace edits.
The host supplies concrete presentation and edit-application adapters; the
controller does not own the clients or their process lifecycle.

**Editor LSP actions**:
The workspace-aware preparation of language-service actions initiated by an
editor. Rename requires pending changes from open documents to be flushed
before dispatch; code actions use the editor's current diagnostics. Results
remain presentation concerns of the active window.

**Document-bound LSP result**:
A language-service result that is safe to apply only while the target URI and
document version still identify the active editor state. Formatting and
outline symbols use this coherence rule; stale asynchronous results are
discarded.

**Workspace edit application**:
The coordinated mutation of open editor documents and closed workspace files
from one language-service edit. It validates all targets before mutation and
keeps presentation of failures in the application shell.

**Completion routing**:
The policy that accepts completion results for the active document, combines
language-service items with local snippets, and presents them through the
editor completer. Results for another tab or a disabled language service are
discarded.

**References routing**:
The policy that accepts a references result only when its URI and document
version still identify the active editor. Accepted results populate the
references presentation; stale results are discarded rather than replacing
the active document's view.

**Code action routing**:
The policy that turns language-service code actions into executable menu
entries. Workspace edits are delegated to workspace edit application, while
commands are offered only when the language service advertises command
execution.

**Location navigation**:
The shared policy for moving from a path or local URI to an editor location.
It opens the target document before positioning the active editor and rejects
remote locations that cannot identify a workspace file.

**Editor chrome**:
The presentation state derived from the active editor: find/replace target,
breadcrumbs, cursor position, language label, window title, and outline
request identity.

`EditorChromeController` is the explicit collaborator for modules that need to
refresh this state. Session lifecycle, editor interaction, and workspace panel
presentation decide when a refresh is needed; they do not duplicate the
derivation policy or route it through a generic `MainWindow` callback.

**Editor workspace presentation**:
The shell presentation mode for the central workspace. It chooses between the
welcome surface and the editor surface from the materialized tab set, keeps
breadcrumbs and find/replace visibility coherent with that mode, and routes
find commands to the active editor. It does not own editor sessions or the
find algorithm.

**Editor workspace presentation controller**:
The module that owns editor-workspace presentation transitions and find-command
routing. It receives the tab and presentation surfaces at its seam, while the
host remains responsible for constructing editors and handling document
lifecycle.

When reducing `MainWindow`, prefer extracting a policy cluster with a focused
public interface and a test that crosses that interface. Avoid generic
coordinators whose only purpose is to relocate widget calls.
