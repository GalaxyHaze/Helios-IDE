#include "LspClient.h"

#include <QFileInfo>

LspClient::LspClient(QObject *parent)
    : QObject(parent), m_transport(nullptr),
      m_requestSender({[this](const QJsonObject &message) {
                         return sendMessage(message);
                       },
                       [this]() { return isRunning(); },
                       [this](const QString &message) { emit logMessage(message); }}),
      m_documentProtocol(
          {[this]() { return isReady(); },
           [this](const QJsonObject &message) {
               return sendMessage(message);
           },
           [this](const QString &uri) { cancelRequestsForUri(uri); }}),
      m_featureRequestRouter(
          {[this](const QString &method, const QJsonObject &params,
                  const QString &uri, int version, bool cancellable,
                  std::function<void(const QJsonObject &)> callback) {
              return sendRequest(method, params, uri, version, cancellable,
                                 std::move(callback));
          }},
          this),
      m_serverMessageDispatcher(
          {[this](const QJsonObject &message) {
               return sendMessage(message);
           },
           [this]() { return isRunning(); },
           [this](const QString &uri, int version) {
               return m_documentProtocol.isCurrentDocument(uri, version);
           },
           [this](const QJsonObject &params) {
               return m_serverRequestHandlers.applyWorkspaceEdit
                          ? m_serverRequestHandlers.applyWorkspaceEdit(params)
                          : QJsonObject{};
           },
           [this](const QJsonObject &params) {
               return m_serverRequestHandlers.showMessageRequest
                          ? m_serverRequestHandlers.showMessageRequest(params)
                          : QJsonValue::Null;
           },
           [this](const QJsonArray &items) {
               if (m_serverRequestHandlers.configuration)
                   return m_serverRequestHandlers.configuration(items);
               QJsonArray values;
               for (int index = 0; index < items.size(); ++index)
                   values.append(QJsonValue::Null);
               return values;
           },
           [this](const QJsonArray &registrations) {
               registerDynamicCapabilities(registrations);
           },
           [this](const QJsonArray &registrations) {
               unregisterDynamicCapabilities(registrations);
           },
           },
          this),
      m_sessionLifecycle(
          {[this](const QJsonObject &message) {
               return sendMessage(message);
           },
           [this](const QString &method, const QJsonObject &params,
                  const QString &uri, int version, bool cancellable,
                  std::function<void(const QJsonObject &)> callback) {
               return sendRequest(method, params, uri, version, cancellable,
                                  std::move(callback));
           },
           [this]() { return m_transport.hasProcess(); },
           [this]() { return m_transport.isRunning(); },
           [this](const QString &path) { return m_transport.start(path); },
           [this]() { m_transport.terminate(); },
           [this]() { m_transport.kill(); },
           [this]() {
               m_capabilities = {};
               m_dynamicCapabilities.clear();
               m_dynamicCapabilityMethods.clear();
               m_documentProtocol.clear();
               m_requestSender.clear();
               m_stderrPartial.clear();
               m_stderrLines.clear();
           },
           [this](const QJsonObject &caps) { parseServerCapabilities(caps); }}) {
  connect(&m_transport, &LspProcessTransport::started, &m_sessionLifecycle,
          &LspSessionLifecycle::processStarted);
  connect(&m_transport, &LspProcessTransport::messageReceived, this,
          &LspClient::dispatchMessage);
  connect(&m_transport, &LspProcessTransport::decodeError, this,
          &LspClient::logMessage);
  connect(&m_transport, &LspProcessTransport::inputTooLarge, this, [this]() {
    emit serverError(QStringLiteral("LSP input buffer exceeded 64 MB"));
  });
  connect(&m_transport, &LspProcessTransport::protocolError, this,
          [this](const QString &message) { emit serverError(message); });
  connect(&m_transport, &LspProcessTransport::stderrChunk, this,
          &LspClient::recordStderr);
  connect(&m_transport, &LspProcessTransport::processError, this,
          [this](const QString &message) {
            emit serverError(QStringLiteral("LSP process error: ") + message);
          });
  connect(&m_transport, &LspProcessTransport::finished, this,
          &LspClient::onProcessFinished);
  connect(&m_sessionLifecycle, &LspSessionLifecycle::initialized, this,
          &LspClient::initialized);
  connect(&m_sessionLifecycle, &LspSessionLifecycle::serverError, this,
          &LspClient::serverError);
  connect(&m_sessionLifecycle, &LspSessionLifecycle::serverStopped, this,
          &LspClient::serverStopped);
  connect(&m_sessionLifecycle, &LspSessionLifecycle::processStopped, this,
          &LspClient::processStopped);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::completionResults, this,
          &LspClient::completionResults);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::completionResolved, this,
          &LspClient::completionResolved);
  connect(&m_featureRequestRouter, &LspFeatureRequestRouter::hoverResult, this,
          &LspClient::hoverResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::definitionResult, this,
          &LspClient::definitionResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::declarationResult, this,
          &LspClient::declarationResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::implementationResult, this,
          &LspClient::implementationResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::referencesResult, this,
          &LspClient::referencesResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::documentHighlightsResult, this,
          &LspClient::documentHighlightsResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::signatureHelpResult, this,
          &LspClient::signatureHelpResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::semanticTokensResult, this,
          &LspClient::semanticTokensResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::formattingResult, this,
          &LspClient::formattingResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::documentSymbolsResult, this,
          &LspClient::documentSymbolsResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::foldingRangesResult, this,
          &LspClient::foldingRangesResult);
  connect(&m_featureRequestRouter, &LspFeatureRequestRouter::renameResult, this,
          &LspClient::renameResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::codeActionsResult, this,
          &LspClient::codeActionsResult);
  connect(&m_featureRequestRouter,
          &LspFeatureRequestRouter::commandResult, this,
          &LspClient::commandResult);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::diagnosticsReceived, this,
          &LspClient::diagnosticsReceived);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::logMessage, this,
          &LspClient::logMessage);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::showMessage, this,
          &LspClient::showMessage);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::saveAllRequested, this,
          &LspClient::saveAllRequested);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::processOutputReceived, this,
          &LspClient::processOutputReceived);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::processExitReceived, this,
          &LspClient::processExitReceived);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::workDoneProgressReceived, this,
          &LspClient::workDoneProgressReceived);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::frontendStatusReceived, this,
          &LspClient::frontendStatusReceived);
  connect(&m_serverMessageDispatcher,
          &LspServerMessageDispatcher::metricsReceived, this,
          &LspClient::metricsReceived);
}

LspClient::~LspClient() = default;

#ifdef HELIOS_UNIT_TESTING
void LspClient::waitForFinishedForTesting(int timeoutMs) {
  m_transport.waitForFinishedForTesting(timeoutMs);
}

void LspClient::setReadyForTesting(bool ready) {
  m_sessionLifecycle.setReadyForTesting(ready);
}
#endif

bool LspClient::start(const LspStartOptions &options) {
  if (!QFileInfo::exists(options.serverPath)) {
    emit serverError(QStringLiteral("LSP server not found: ") +
                     options.serverPath);
    return false;
  }
  return m_sessionLifecycle.start(options, m_transport.hasProcess());
}

bool LspClient::start(const QString &serverPath, const QString &stdlibPath,
                      const QString &workspaceRoot, const QString &initMode) {
  return start(
      LspStartOptions{serverPath, stdlibPath, workspaceRoot, initMode});
}

void LspClient::stop() {
  m_sessionLifecycle.stop();
}

bool LspClient::isRunning() const {
  return m_transport.isRunning();
}
bool LspClient::isReady() const { return m_sessionLifecycle.isReady(); }
bool LspClient::supports(Capability capability) const {
  bool supported = false;
  switch (capability) {
  case Capability::Completion:
    supported = m_capabilities.completionProvider;
    break;
  case Capability::Hover:
    supported = m_capabilities.hoverProvider;
    break;
  case Capability::SignatureHelp:
    supported = m_capabilities.signatureHelpProvider;
    break;
  case Capability::Definition:
    supported = m_capabilities.definitionProvider;
    break;
  case Capability::Implementation:
    supported = m_capabilities.implementationProvider;
    break;
  case Capability::Declaration:
    supported = m_capabilities.declarationProvider;
    break;
  case Capability::References:
    supported = m_capabilities.referencesProvider;
    break;
  case Capability::DocumentHighlight:
    supported = m_capabilities.documentHighlightProvider;
    break;
  case Capability::Rename:
    supported = m_capabilities.renameProvider;
    break;
  case Capability::DocumentSymbol:
    supported = m_capabilities.documentSymbolProvider;
    break;
  case Capability::Formatting:
    supported = m_capabilities.formattingProvider;
    break;
  case Capability::FoldingRange:
    supported = m_capabilities.foldingRangeProvider;
    break;
  case Capability::CodeAction:
    supported = m_capabilities.codeActionProvider;
    break;
  case Capability::SemanticTokens:
    supported = m_capabilities.semanticTokensProvider;
    break;
  case Capability::ExecuteCommand:
    supported = m_capabilities.executeCommandProvider;
    break;
  }
  return supported ||
         m_dynamicCapabilities.contains(capabilityMethod(capability));
}

void LspClient::setServerRequestHandlers(ServerRequestHandlers handlers)
{
  m_serverRequestHandlers = std::move(handlers);
}
int LspClient::documentVersion(const QString &uri) const {
  return m_documentProtocol.documentVersion(uri);
}

void LspClient::parseServerCapabilities(const QJsonObject &caps) {
  m_capabilities = LspServerCapabilities::fromJson(caps);
}

QString LspClient::capabilityMethod(Capability capability)
{
  switch (capability)
  {
  case Capability::Completion:
    return QStringLiteral("textDocument/completion");
  case Capability::Hover:
    return QStringLiteral("textDocument/hover");
  case Capability::SignatureHelp:
    return QStringLiteral("textDocument/signatureHelp");
  case Capability::Definition:
    return QStringLiteral("textDocument/definition");
  case Capability::Implementation:
    return QStringLiteral("textDocument/implementation");
  case Capability::Declaration:
    return QStringLiteral("textDocument/declaration");
  case Capability::References:
    return QStringLiteral("textDocument/references");
  case Capability::DocumentHighlight:
    return QStringLiteral("textDocument/documentHighlight");
  case Capability::Rename:
    return QStringLiteral("textDocument/rename");
  case Capability::DocumentSymbol:
    return QStringLiteral("textDocument/documentSymbol");
  case Capability::Formatting:
    return QStringLiteral("textDocument/formatting");
  case Capability::FoldingRange:
    return QStringLiteral("textDocument/foldingRange");
  case Capability::CodeAction:
    return QStringLiteral("textDocument/codeAction");
  case Capability::SemanticTokens:
    return QStringLiteral("textDocument/semanticTokens/full");
  case Capability::ExecuteCommand:
    return QStringLiteral("workspace/executeCommand");
  }
  return {};
}

void LspClient::registerDynamicCapabilities(
    const QJsonArray &registrations)
{
  for (const auto &value : registrations)
  {
    const QJsonObject registration = value.toObject();
    const QString method =
        registration.value(QStringLiteral("method")).toString();
    if (method.isEmpty())
      continue;

    m_dynamicCapabilities.insert(method);
    const QString id =
        registration.value(QStringLiteral("id")).toString();
    if (!id.isEmpty())
      m_dynamicCapabilityMethods.insert(id, method);
  }
}

void LspClient::unregisterDynamicCapabilities(
    const QJsonArray &registrations)
{
  for (const auto &value : registrations)
  {
    const QJsonObject registration = value.toObject();
    QString method = registration.value(QStringLiteral("method")).toString();
    const QString id = registration.value(QStringLiteral("id")).toString();
    if (method.isEmpty() && !id.isEmpty())
      method = m_dynamicCapabilityMethods.value(id);
    if (!id.isEmpty())
      m_dynamicCapabilityMethods.remove(id);
    if (method.isEmpty())
      continue;

    bool stillRegistered = false;
    for (auto it = m_dynamicCapabilityMethods.cbegin();
         it != m_dynamicCapabilityMethods.cend(); ++it) {
      if (it.value() == method) {
        stillRegistered = true;
        break;
      }
    }
    if (!stillRegistered)
      m_dynamicCapabilities.remove(method);
  }
}

bool LspClient::sendMessage(const QJsonObject &msg) {
  return m_transport.writeMessage(msg);
}

qint64
LspClient::sendRequest(const QString &method, const QJsonObject &params,
                       const QString &uri, int version, bool cancellable,
                       std::function<void(const QJsonObject &)> callback) {
  return m_requestSender.send(method, params, uri, version, cancellable,
                              m_sessionLifecycle.isReady(), std::move(callback));
}

void LspClient::cancelRequest(qint64 id) {
  m_requestSender.cancel(id);
}

void LspClient::cancelRequestsForUri(const QString &uri) {
  m_requestSender.cancelForUri(uri);
}

void LspClient::openDocument(const QString &uri, const QString &languageId,
                             const QString &text, int version) {
  m_documentProtocol.openDocument(uri, languageId, text, version);
}

bool LspClient::changeDocument(const QString &uri,
                               const QList<LspTextChange> &changes,
                               int version) {
  return m_documentProtocol.changeDocument(uri, changes, version);
}

bool LspClient::changeDocumentFull(const QString &uri,
                                   const QString &fullText, int version) {
  return m_documentProtocol.changeDocumentFull(uri, fullText, version);
}

void LspClient::closeDocument(const QString &uri) {
  m_documentProtocol.closeDocument(uri);
}

void LspClient::saveDocument(const QString &uri) {
  m_documentProtocol.saveDocument(uri);
}

void LspClient::notifyConfigurationChanged(const QJsonValue &settings)
{
    if (!isReady())
        return;
    sendMessage({{"jsonrpc", "2.0"},
                 {"method", "workspace/didChangeConfiguration"},
                 {"params", QJsonObject{{"settings", settings}}}});
}

void LspClient::cancelInteractiveRequestsForUri(const QString &uri)
{
    if (!uri.isEmpty())
        m_requestSender.cancelCancellableForUri(uri);
}

void LspClient::requestCompletion(const QString &uri, int version,
                                  const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::Completion, uri, version,
        pos);
}

void LspClient::requestHover(const QString &uri, int version,
                             const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::Hover, uri, version, pos);
}

void LspClient::requestReferences(const QString &uri, int version,
                                  const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::References, uri, version,
        pos);
}

void LspClient::requestDocumentHighlight(const QString &uri, int version,
                                         const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::DocumentHighlight, uri,
        version, pos);
}

void LspClient::requestSignatureHelp(const QString &uri, int version,
                                     const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::SignatureHelp, uri, version,
        pos);
}

void LspClient::requestDefinition(const QString &uri, int version,
                                  const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::Definition, uri, version,
        pos);
}

void LspClient::requestDeclaration(const QString &uri, int version,
                                   const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::Declaration, uri, version,
        pos);
}

void LspClient::requestImplementation(const QString &uri, int version,
                                      const LspPosition &pos)
{
    m_featureRequestRouter.requestPosition(
        LspFeatureRequestRouter::PositionFeature::Implementation, uri,
        version, pos);
}

void LspClient::requestSemanticTokens(const QString &uri, int version)
{
    m_featureRequestRouter.requestDocument(
        LspFeatureRequestRouter::DocumentFeature::SemanticTokens, uri,
        version);
}

void LspClient::requestFormatting(const QString &uri, int version)
{
    m_featureRequestRouter.requestDocument(
        LspFeatureRequestRouter::DocumentFeature::Formatting, uri, version);
}

void LspClient::requestDocumentSymbols(const QString &uri, int version)
{
    m_featureRequestRouter.requestDocument(
        LspFeatureRequestRouter::DocumentFeature::DocumentSymbols, uri,
        version);
}

void LspClient::requestFoldingRanges(const QString &uri, int version)
{
    m_featureRequestRouter.requestDocument(
        LspFeatureRequestRouter::DocumentFeature::FoldingRanges, uri, version);
}

void LspClient::requestRename(const QString &uri, int version,
                              const LspPosition &pos, const QString &newName)
{
    m_featureRequestRouter.requestRename(uri, version, pos, newName);
}

void LspClient::requestCodeActions(const QString &uri, int version,
                                   const LspRange &range,
                                   const QList<LspDiagnostic> &diagnostics)
{
    m_featureRequestRouter.requestCodeActions(uri, version, range, diagnostics);
}

qint64 LspClient::executeWorkspaceCommand(
    const QString &command, const QJsonValue &args,
    std::function<void(const QJsonObject &)> callback)
{
    return m_featureRequestRouter.executeWorkspaceCommand(command, args,
                                                          std::move(callback));
}

void LspClient::resolveCompletion(const QString &uri, int version,
                                  const QJsonObject &item)
{
    m_featureRequestRouter.resolveCompletion(uri, version, item);
}

void LspClient::recordStderr(const QByteArray &chunk) {
  m_stderrPartial += chunk;
  QList<QByteArray> lines = m_stderrPartial.split('\n');
  m_stderrPartial = lines.takeLast();
  for (const QByteArray &line : lines) {
    QString s = QString::fromUtf8(line).trimmed();
    if (!s.isEmpty()) {
      m_stderrLines.append(s);
      if (m_stderrLines.size() > 50)
        m_stderrLines.removeFirst();
    }
  }
}

void LspClient::dispatchMessage(const QJsonObject &message) {
  if (message.contains("id") &&
      (message.contains("result") || message.contains("error")))
    m_requestSender.handleResponse(
        message, [this](const QString &uri, int version) {
          return m_documentProtocol.isCurrentDocument(uri, version);
        });
  else
    m_serverMessageDispatcher.dispatch(message);
}

void LspClient::onProcessFinished(const LspProcessResult &result) {
  m_sessionLifecycle.processFinished(result,
                                    unexpectedProcessExitMessage(result));
}

QString LspClient::unexpectedProcessExitMessage(
    const LspProcessResult &result) const {
  QString message =
      QString("LSP process exited (code %1, %2)")
          .arg(result.exitCode)
          .arg(result.crashed ? "crash" : "normal");
  if (!m_stderrPartial.trimmed().isEmpty())
    message += "\n" + QString::fromUtf8(m_stderrPartial).trimmed();
  if (!m_stderrLines.isEmpty())
    message += "\n" + m_stderrLines.join("\n");
  return message;
}
