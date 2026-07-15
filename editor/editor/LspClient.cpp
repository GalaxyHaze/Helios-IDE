#include "LspClient.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>

namespace {
QString requestKey(const QString &method, const QString &uri) {
  return method + QLatin1Char('\n') + uri;
}

bool providerEnabled(const QJsonObject &caps, const char *name) {
  return caps.contains(QLatin1String(name)) &&
         !caps.value(QLatin1String(name)).isNull() &&
         caps.value(QLatin1String(name)).toBool(true);
}

QJsonObject positionObject(const LspPosition &position) {
  return {{"line", position.line}, {"character", position.character}};
}

QJsonObject rangeObject(const LspRange &range) {
  return {{"start", positionObject(range.start)},
          {"end", positionObject(range.end)}};
}
} // namespace

LspClient::LspClient(QObject *parent) : QObject(parent) {}

LspClient::~LspClient() {
  if (m_process) {
    m_process->disconnect(this);
    m_process->kill();
  }
}

bool LspClient::start(const QString &serverPath, const QString &stdlibPath,
                      const QString &workspaceRoot) {
  if (!QFileInfo::exists(serverPath)) {
    emit serverError(QStringLiteral("LSP server not found: ") + serverPath);
    return false;
  }
  m_serverPath = serverPath;
  m_stdlibPath = stdlibPath;
  m_workspaceRoot = workspaceRoot;
  if (m_process) {
    m_startPending = true;
    stop();
    return true;
  }
  launch();
  return true;
}

void LspClient::launch() {
  resetSessionState();
  m_process = new QProcess(this);
  m_process->setProcessChannelMode(QProcess::SeparateChannels);
  connect(m_process, &QProcess::started, this, &LspClient::onProcessStarted);
  connect(m_process, &QProcess::readyReadStandardOutput, this,
          &LspClient::onReadyRead);
  connect(m_process, &QProcess::readyReadStandardError, this,
          &LspClient::onReadyReadError);
  connect(m_process, &QProcess::errorOccurred, this,
          &LspClient::onProcessError);
  connect(m_process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &LspClient::onProcessFinished);
  m_process->start(m_serverPath, {}, QIODevice::ReadWrite);
}

void LspClient::resetSessionState() {
  m_initialized = false;
  m_shutdownRequested = false;
  m_exitSent = false;
  m_stopping = false;
  m_syncKind = 1;
  m_buffer.clear();
  m_stderrPartial.clear();
  m_stderrLines.clear();
  m_documents.clear();
  m_pendingRequests.clear();
  m_replaceableRequests.clear();
  m_hasCompletionProvider = m_hasHoverProvider = m_hasDefinitionProvider =
      false;
  m_hasImplementationProvider = m_hasDeclarationProvider =
      m_hasReferencesProvider = false;
  m_hasDocumentHighlightProvider = m_hasRenameProvider =
      m_hasDocumentSymbolProvider = false;
  m_hasFormattingProvider = m_hasFoldingRangeProvider =
      m_hasCodeActionProvider = false;
  m_hasSemanticTokensProvider = false;
}

void LspClient::stop() {
  if (!m_process || m_stopping)
    return;
  m_stopping = true;
  if (m_process->state() == QProcess::NotRunning) {
    return;
  }
  if (m_initialized) {
    beginShutdown();
  } else {
    QJsonObject exit{{"jsonrpc", "2.0"}, {"method", "exit"}};
    sendMessage(exit);
    finishShutdownEscalation();
  }
}

void LspClient::beginShutdown() {
  if (m_shutdownRequested)
    return;
  m_shutdownRequested = true;
  sendRequest("shutdown", {}, {}, -1, false, [this](const QJsonObject &) {
    if (!m_process || m_exitSent)
      return;
    m_exitSent = true;
    sendMessage({{"jsonrpc", "2.0"}, {"method", "exit"}});
    finishShutdownEscalation();
  });
}

void LspClient::finishShutdownEscalation() {
  if (!m_process)
    return;
  if (!m_shutdownTimer) {
    m_shutdownTimer = new QTimer(this);
    m_shutdownTimer->setSingleShot(true);
    connect(m_shutdownTimer, &QTimer::timeout, this, [this]() {
      if (!m_process || m_process->state() == QProcess::NotRunning)
        return;
      if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_shutdownTimer->start(500);
      } else {
        m_process->kill();
      }
    });
  }
  m_shutdownTimer->start(1000);
}

bool LspClient::isRunning() const {
  return m_process && m_process->state() != QProcess::NotRunning;
}
bool LspClient::isReady() const { return isRunning() && m_initialized; }
int LspClient::documentSyncKind() const { return m_syncKind; }
int LspClient::documentVersion(const QString &uri) const {
  return m_documents.value(uri).version;
}

void LspClient::onProcessStarted() {
  QJsonObject textDocument{
      {"completion",
       QJsonObject{
           {"completionItem",
            QJsonObject{
                {"snippetSupport", true},
                {"resolveSupport",
                 QJsonObject{{"properties",
                              QJsonArray{"detail", "documentation"}}}}}}}},
      {"hover",
       QJsonObject{{"contentFormat", QJsonArray{"markdown", "plaintext"}}}},
      {"definition", QJsonObject{}},
      {"declaration", QJsonObject{}},
      {"implementation", QJsonObject{}},
      {"references", QJsonObject{}},
      {"documentHighlight", QJsonObject{}},
      {"documentSymbol",
       QJsonObject{{"hierarchicalDocumentSymbolSupport", true}}},
      {"rename", QJsonObject{}},
      {"formatting", QJsonObject{}},
      {"foldingRange", QJsonObject{}},
      {"codeAction", QJsonObject{}},
      {"semanticTokens", QJsonObject{{"requests", QJsonObject{{"full", true}}},
                                     {"formats", QJsonArray{"relative"}}}}};
  QJsonObject params{
      {"processId", QJsonValue::Null},
      {"capabilities",
       QJsonObject{
           {"textDocument", textDocument},
           {"experimental",
            QJsonObject{{"zith", QJsonObject{{"requestSaveAll", true}}}}}}}};
  const QString root =
      m_workspaceRoot.isEmpty() ? QDir::currentPath() : m_workspaceRoot;
  params["rootUri"] = QUrl::fromLocalFile(root).toString();
  params["rootPath"] = root;
  if (!m_stdlibPath.isEmpty())
    params["initializationOptions"] = QJsonObject{{"stdlibPath", m_stdlibPath}};
  sendRequest("initialize", params, {}, -1, false,
              [this](const QJsonObject &response) {
                if (response.contains("error"))
                  return;
                parseServerCapabilities(response.value("result")
                                            .toObject()
                                            .value("capabilities")
                                            .toObject());
                if (!sendMessage({{"jsonrpc", "2.0"},
                                  {"method", "initialized"},
                                  {"params", QJsonObject{}}}))
                  return;
                m_initialized = true;
                emit initialized();
              });
}

void LspClient::parseServerCapabilities(const QJsonObject &caps) {
  m_hasCompletionProvider = providerEnabled(caps, "completionProvider");
  m_hasHoverProvider = providerEnabled(caps, "hoverProvider");
  m_hasDefinitionProvider = providerEnabled(caps, "definitionProvider");
  m_hasDeclarationProvider = providerEnabled(caps, "declarationProvider");
  m_hasImplementationProvider = providerEnabled(caps, "implementationProvider");
  m_hasReferencesProvider = providerEnabled(caps, "referencesProvider");
  m_hasDocumentHighlightProvider =
      providerEnabled(caps, "documentHighlightProvider");
  m_hasDocumentSymbolProvider = providerEnabled(caps, "documentSymbolProvider");
  m_hasRenameProvider = providerEnabled(caps, "renameProvider");
  m_hasFormattingProvider = providerEnabled(caps, "documentFormattingProvider");
  m_hasFoldingRangeProvider = providerEnabled(caps, "foldingRangeProvider");
  m_hasCodeActionProvider = providerEnabled(caps, "codeActionProvider");
  m_hasSemanticTokensProvider = providerEnabled(caps, "semanticTokensProvider");
  const QJsonValue sync = caps.value("textDocumentSync");
  m_syncKind = sync.isObject() ? sync.toObject().value("change").toInt(1)
                               : sync.toInt(1);
}

bool LspClient::sendMessage(const QJsonObject &msg) {
  if (!m_process || m_process->state() != QProcess::Running)
    return false;
  const QByteArray body = QJsonDocument(msg).toJson(QJsonDocument::Compact);
  if (body.size() > MaxMessageSize) {
    emit serverError("Refused LSP message larger than 64 MB");
    return false;
  }
  const QByteArray frame =
      "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
  return m_process->write(frame) == frame.size();
}

qint64 LspClient::nextId() { return ++m_nextId; }

qint64
LspClient::sendRequest(const QString &method, const QJsonObject &params,
                       const QString &uri, int version, bool cancellable,
                       std::function<void(const QJsonObject &)> callback) {
  if (method != QLatin1String("initialize") && !isReady())
    return -1;
  const qint64 id = nextId();
  const QJsonObject message{
      {"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", params}};
  if (cancellable && !uri.isEmpty()) {
    const QString key = requestKey(method, uri);
    if (m_replaceableRequests.contains(key))
      cancelRequest(m_replaceableRequests.value(key));
    m_replaceableRequests.insert(key, id);
  }
  PendingRequest request{id,
                         method,
                         uri,
                         version,
                         cancellable,
                         std::move(callback),
                         new QTimer(this)};
  request.timeout->setSingleShot(true);
  connect(request.timeout, &QTimer::timeout, this,
          [this, id]() { cancelRequest(id); });
  m_pendingRequests.insert(id, request);
  if (!sendMessage(message)) {
    cancelRequest(id);
    return -1;
  }
  request.timeout->start(8000);
  return id;
}

void LspClient::cancelRequest(qint64 id) {
  auto it = m_pendingRequests.find(id);
  if (it == m_pendingRequests.end())
    return;
  const PendingRequest request = it.value();
  if (request.timeout)
    request.timeout->deleteLater();
  m_pendingRequests.erase(it);
  if (request.cancellable)
    m_replaceableRequests.remove(requestKey(request.method, request.uri));
  if (isRunning())
    sendMessage({{"jsonrpc", "2.0"},
                 {"method", "$/cancelRequest"},
                 {"params", QJsonObject{{"id", id}}}});
}

void LspClient::cancelRequestsForUri(const QString &uri) {
  QList<qint64> ids;
  for (auto it = m_pendingRequests.cbegin(); it != m_pendingRequests.cend();
       ++it)
    if (it->uri == uri)
      ids.append(it.key());
  for (qint64 id : ids)
    cancelRequest(id);
}

void LspClient::failPendingRequests(const QString &) {
  for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it)
    if (it->timeout)
      it->timeout->deleteLater();
  m_pendingRequests.clear();
  m_replaceableRequests.clear();
}

void LspClient::openDocument(const QString &uri, const QString &languageId,
                             const QString &text, int version) {
  if (!isReady())
    return;
  m_documents.insert(uri, {version, true});
  sendMessage({{"jsonrpc", "2.0"},
               {"method", "textDocument/didOpen"},
               {"params", QJsonObject{{"textDocument",
                                       QJsonObject{{"uri", uri},
                                                   {"languageId", languageId},
                                                   {"version", version},
                                                   {"text", text}}}}}});
}

void LspClient::changeDocument(const QString &uri,
                               const QList<LspTextChange> &changes,
                               int version) {
  if (!isReady() || changes.isEmpty())
    return;
  m_documents[uri] = {version, true};
  cancelRequestsForUri(uri);
  QJsonArray content;
  for (const LspTextChange &change : changes)
    content.append(QJsonObject{{"range", rangeObject(change.range)},
                               {"text", change.text}});
  sendMessage(
      {{"jsonrpc", "2.0"},
       {"method", "textDocument/didChange"},
       {"params", QJsonObject{{"textDocument",
                               QJsonObject{{"uri", uri}, {"version", version}}},
                              {"contentChanges", content}}}});
}

void LspClient::changeDocumentFull(const QString &uri, const QString &fullText,
                                   int version) {
  if (!isReady())
    return;
  m_documents[uri] = {version, true};
  cancelRequestsForUri(uri);
  sendMessage(
      {{"jsonrpc", "2.0"},
       {"method", "textDocument/didChange"},
       {"params",
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", uri}, {"version", version}}},
            {"contentChanges", QJsonArray{QJsonObject{{"text", fullText}}}}}}});
}

void LspClient::closeDocument(const QString &uri) {
  cancelRequestsForUri(uri);
  m_documents.remove(uri);
  if (!isReady())
    return;
  sendMessage(
      {{"jsonrpc", "2.0"},
       {"method", "textDocument/didClose"},
       {"params", QJsonObject{{"textDocument", QJsonObject{{"uri", uri}}}}}});
}

void LspClient::saveDocument(const QString &uri) {
  if (isReady())
    sendMessage(
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didSave"},
         {"params", QJsonObject{{"textDocument", QJsonObject{{"uri", uri}}}}}});
}

QJsonObject LspClient::positionParams(const QString &uri,
                                      const LspPosition &pos) const {
  return {{"textDocument", QJsonObject{{"uri", uri}}},
          {"position", positionObject(pos)}};
}

#define LSP_POSITION_REQUEST(name, method, signalName)                         \
  void LspClient::name(const QString &uri, int version,                        \
                       const LspPosition &pos) {                               \
    sendRequest(method, positionParams(uri, pos), uri, version, true,          \
                [this, uri, version](const QJsonObject &r) {                   \
                  const QJsonValue result = r.value("result");                 \
                  const QJsonArray locations = result.toArray();               \
                  emit signalName(uri, version,                                \
                                  locationFromJson(                            \
                                      result.isArray() && !locations.isEmpty() \
                                          ? locations.first().toObject()       \
                                          : result.toObject()));               \
                });                                                            \
  }
LSP_POSITION_REQUEST(requestDefinition, "textDocument/definition",
                     definitionResult)
LSP_POSITION_REQUEST(requestDeclaration, "textDocument/declaration",
                     declarationResult)
LSP_POSITION_REQUEST(requestImplementation, "textDocument/implementation",
                     implementationResult)
#undef LSP_POSITION_REQUEST

void LspClient::requestCompletion(const QString &uri, int version,
                                  const LspPosition &pos) {
  sendRequest("textDocument/completion", positionParams(uri, pos), uri, version,
              true, [this, uri, version](const QJsonObject &r) {
                QJsonValue result = r.value("result");
                QJsonArray values =
                    result.isArray()
                        ? result.toArray()
                        : result.toObject().value("items").toArray();
                QList<LspCompletionItem> items;
                for (const QJsonValue &value : values) {
                  QJsonObject o = value.toObject();
                  LspCompletionItem item;
                  item.label = o.value("label").toString();
                  item.kind = o.value("kind").toInt();
                  item.detail = o.value("detail").toString();
                  item.insertText = o.value("insertText").toString(item.label);
                  item.insertTextFormat = o.value("insertTextFormat").toInt(1);
                  item.rawItem = o;
                  items.append(item);
                }
                emit completionResults(uri, version, items);
              });
}

void LspClient::requestHover(const QString &uri, int version,
                             const LspPosition &pos) {
  sendRequest("textDocument/hover", positionParams(uri, pos), uri, version,
              true, [this, uri, version](const QJsonObject &r) {
                LspHoverInfo info;
                QJsonObject v = r.value("result").toObject();
                QJsonValue c = v.value("contents");
                if (c.isString())
                  info.contents = c.toString();
                else if (c.isObject())
                  info.contents = c.toObject().value("value").toString();
                else
                  for (const QJsonValue &x : c.toArray())
                    info.contents +=
                        (info.contents.isEmpty() ? QString() : "\n\n") +
                        (x.isString() ? x.toString()
                                      : x.toObject().value("value").toString());
                info.range = rangeFromJson(v.value("range").toObject());
                emit hoverResult(uri, version, info);
              });
}

void LspClient::requestReferences(const QString &uri, int version,
                                  const LspPosition &pos) {
  QJsonObject p = positionParams(uri, pos);
  p["context"] = QJsonObject{{"includeDeclaration", true}};
  sendRequest("textDocument/references", p, uri, version, true,
              [this, uri, version](const QJsonObject &r) {
                emit referencesResult(uri, version,
                                      locationsFromJson(r.value("result")));
              });
}

void LspClient::requestDocumentHighlight(const QString &uri, int version,
                                         const LspPosition &pos) {
  sendRequest("textDocument/documentHighlight", positionParams(uri, pos), uri,
              version, true, [this, uri, version](const QJsonObject &r) {
                QList<LspRange> ranges;
                for (const QJsonValue &v : r.value("result").toArray())
                  ranges.append(
                      rangeFromJson(v.toObject().value("range").toObject()));
                emit documentHighlightsResult(uri, version, ranges);
              });
}

void LspClient::requestSignatureHelp(const QString &uri, int version,
                                     const LspPosition &pos) {
  sendRequest("textDocument/signatureHelp", positionParams(uri, pos), uri,
              version, true, [this, uri, version](const QJsonObject &r) {
                LspSignatureHelp h;
                QJsonObject o = r.value("result").toObject();
                QJsonArray s = o.value("signatures").toArray();
                if (!s.isEmpty()) {
                  QJsonObject sig = s.first().toObject();
                  h.activeSignature = sig.value("label").toString();
                  for (const QJsonValue &p : sig.value("parameters").toArray())
                    h.parameters.append(p.toObject().value("label").toString());
                }
                h.activeParameter = o.value("activeParameter").toInt();
                emit signatureHelpResult(uri, version, h);
              });
}

void LspClient::requestSemanticTokens(const QString &uri, int version) {
  sendRequest("textDocument/semanticTokens/full",
              {{"textDocument", QJsonObject{{"uri", uri}}}}, uri, version, true,
              [this, uri, version](const QJsonObject &r) {
                emit semanticTokensResult(
                    uri, version,
                    r.value("result").toObject().value("data").toArray());
              });
}
void LspClient::requestFormatting(const QString &uri, int version) {
  sendRequest(
      "textDocument/formatting",
      {{"textDocument", QJsonObject{{"uri", uri}}},
       {"options", QJsonObject{{"tabSize", 4}, {"insertSpaces", true}}}},
      uri, version, false, [this, uri, version](const QJsonObject &r) {
        QList<QPair<LspRange, QString>> e;
        for (const QJsonValue &v : r.value("result").toArray()) {
          QJsonObject o = v.toObject();
          e.append({rangeFromJson(o.value("range").toObject()),
                    o.value("newText").toString()});
        }
        emit formattingResult(uri, version, e);
      });
}
void LspClient::requestDocumentSymbols(const QString &uri, int version) {
  sendRequest("textDocument/documentSymbol",
              {{"textDocument", QJsonObject{{"uri", uri}}}}, uri, version, true,
              [this, uri, version](const QJsonObject &r) {
                emit documentSymbolsResult(uri, version,
                                           r.value("result").toArray());
              });
}
void LspClient::requestFoldingRanges(const QString &uri, int version) {
  sendRequest("textDocument/foldingRange",
              {{"textDocument", QJsonObject{{"uri", uri}}}}, uri, version, true,
              [this, uri, version](const QJsonObject &r) {
                emit foldingRangesResult(uri, version,
                                         r.value("result").toArray());
              });
}

void LspClient::requestRename(const QString &uri, int version,
                              const LspPosition &pos, const QString &newName) {
  QJsonObject p = positionParams(uri, pos);
  p["newName"] = newName;
  sendRequest("textDocument/rename", p, uri, version, false,
              [this, uri, version](const QJsonObject &r) {
                emit renameResult(uri, version, r.value("result").toObject());
              });
}
void LspClient::requestCodeActions(const QString &uri, int version,
                                   const LspRange &range,
                                   const QList<LspDiagnostic> &diagnostics) {
  QJsonArray ds;
  for (const auto &d : diagnostics)
    ds.append(QJsonObject{{"range", rangeObject(d.range)},
                          {"severity", d.severity},
                          {"message", d.message},
                          {"source", d.source}});
  QJsonObject p{
      {"textDocument", QJsonObject{{"uri", uri}}},
      {"range", rangeObject(range)},
      {"context", QJsonObject{{"diagnostics", ds},
                              {"only", QJsonArray{"refactor.extract"}}}}};
  sendRequest("textDocument/codeAction", p, uri, version, false,
              [this, uri, version](const QJsonObject &r) {
                emit codeActionsResult(uri, version,
                                       r.value("result").toArray());
              });
}
void LspClient::resolveCompletion(const QString &uri, int version,
                                  const QJsonObject &item) {
  sendRequest("completionItem/resolve", item, uri, version, true,
              [this, uri, version](const QJsonObject &r) {
                QJsonObject o = r.value("result").toObject();
                LspCompletionItem i;
                i.label = o.value("label").toString();
                i.detail = o.value("detail").toString();
                i.insertText = o.value("insertText").toString(i.label);
                i.insertTextFormat = o.value("insertTextFormat").toInt(1);
                QJsonValue d = o.value("documentation");
                if (d.isString())
                  i.detail += "\n" + d.toString();
                else if (d.isObject())
                  i.detail += "\n" + d.toObject().value("value").toString();
                i.rawItem = o;
                emit completionResolved(uri, version, i);
              });
}

LspRange LspClient::rangeFromJson(const QJsonObject &r) {
  const QJsonObject s = r.value("start").toObject(),
                    e = r.value("end").toObject();
  return {{s.value("line").toInt(), s.value("character").toInt()},
          {e.value("line").toInt(), e.value("character").toInt()}};
}
LspLocation LspClient::locationFromJson(const QJsonObject &o) {
  LspLocation l;
  if (o.contains("targetUri")) {
    l.uri = o.value("targetUri").toString();
    l.range = rangeFromJson(o.value("targetSelectionRange").toObject());
    if (l.range.start.line == 0 && l.range.start.character == 0 &&
        l.range.end.line == 0 && l.range.end.character == 0)
      l.range = rangeFromJson(o.value("targetRange").toObject());
  } else {
    l.uri = o.value("uri").toString();
    l.range = rangeFromJson(o.value("range").toObject());
  }
  return l;
}
QList<LspLocation> LspClient::locationsFromJson(const QJsonValue &v) {
  QList<LspLocation> result;
  if (v.isArray())
    for (const QJsonValue &x : v.toArray())
      result.append(locationFromJson(x.toObject()));
  else if (v.isObject())
    result.append(locationFromJson(v.toObject()));
  return result;
}

void LspClient::onReadyRead() {
  if (!m_process)
    return;
  m_buffer.append(m_process->readAllStandardOutput());
  if (m_buffer.size() > MaxMessageSize + 8192) {
    emit serverError("LSP input buffer exceeded 64 MB");
    m_buffer.clear();
    return;
  }
  processBuffer();
}
void LspClient::onReadyReadError() {
  if (m_process)
    recordStderr(m_process->readAllStandardError());
}
void LspClient::recordStderr(const QByteArray &chunk) {
  m_stderrPartial += chunk;
  QList<QByteArray> lines = m_stderrPartial.split('\n');
  m_stderrPartial = lines.takeLast();
  for (QByteArray line : lines) {
    QString s = QString::fromUtf8(line).trimmed();
    if (!s.isEmpty()) {
      m_stderrLines.append(s);
      if (m_stderrLines.size() > 50)
        m_stderrLines.removeFirst();
    }
  }
}

void LspClient::processBuffer() {
  while (true) {
    const qsizetype end = m_buffer.indexOf("\r\n\r\n");
    if (end < 0)
      return;
    if (end > 8192) {
      m_buffer.remove(0, end + 4);
      emit logMessage("Discarded oversized LSP header");
      continue;
    }
    bool found = false, ok = false;
    qint64 length = 0;
    for (const QByteArray &line : m_buffer.left(end).split('\n')) {
      const QByteArray t = line.trimmed();
      const qsizetype colon = t.indexOf(':');
      if (colon > 0 &&
          t.left(colon).compare("Content-Length", Qt::CaseInsensitive) == 0) {
        found = true;
        length = t.mid(colon + 1).trimmed().toLongLong(&ok);
        break;
      }
    }
    if (!found || !ok || length < 0 || length > MaxMessageSize) {
      m_buffer.remove(0, end + 4);
      emit logMessage("Discarded malformed LSP frame header");
      continue;
    }
    const qint64 start = end + 4;
    if (m_buffer.size() < start + length)
      return;
    QByteArray body = m_buffer.mid(start, length);
    m_buffer.remove(0, start + length);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
      continue;
    QJsonObject message = doc.object();
    if (message.contains("id") &&
        (message.contains("result") || message.contains("error")))
      handleResponse(message);
    else if (message.contains("method"))
      handleNotification(message);
  }
}

void LspClient::handleResponse(const QJsonObject &message) {
  const qint64 id = message.value("id").toVariant().toLongLong();
  auto it = m_pendingRequests.find(id);
  if (it == m_pendingRequests.end())
    return;
  PendingRequest request = it.value();
  if (request.timeout)
    request.timeout->deleteLater();
  m_pendingRequests.erase(it);
  if (request.cancellable)
    m_replaceableRequests.remove(requestKey(request.method, request.uri));
  if (!request.uri.isEmpty() &&
      (!m_documents.contains(request.uri) ||
       m_documents.value(request.uri).version != request.version))
    return;
  QJsonObject error = message.value("error").toObject();
  if (!error.isEmpty()) {
    const int code = error.value("code").toInt();
    if (code != -32800 && code != -32801)
      emit logMessage(
          QString("LSP %1 failed: %2")
              .arg(request.method, error.value("message").toString()));
    return;
  }
  if (request.callback)
    request.callback(message);
}

void LspClient::handleNotification(const QJsonObject &message) {
  const QString method = message.value("method").toString();
  const QJsonObject p = message.value("params").toObject();
  if (method == "textDocument/publishDiagnostics") {
    const QString uri = p.value("uri").toString();
    const int version = p.contains("version") ? p.value("version").toInt() : -1;
    if (version >= 0 && (!m_documents.contains(uri) ||
                         m_documents.value(uri).version != version))
      return;
    QList<LspDiagnostic> ds;
    for (const QJsonValue &v : p.value("diagnostics").toArray()) {
      QJsonObject d = v.toObject();
      LspDiagnostic x;
      x.range = rangeFromJson(d.value("range").toObject());
      x.severity = d.value("severity").toInt();
      x.message = d.value("message").toString();
      x.source = d.value("source").toString();
      ds.append(x);
    }
    emit diagnosticsReceived(uri, version, ds);
  } else if (method == "window/logMessage")
    emit logMessage(p.value("message").toString());
  else if (method == "window/showMessage") {
    emit logMessage(p.value("message").toString());
    emit showMessage(p.value("message").toString());
  } else if (method == "zith/requestSaveAll")
    emit saveAllRequested();
}

void LspClient::onProcessError(QProcess::ProcessError) {
  if (!m_process || m_process->state() != QProcess::NotRunning)
    return;
  emit serverError("LSP process error: " + m_process->errorString());
  // FailedToStart is not guaranteed to emit finished(). Defer cleanup so a
  // normal finished signal, if one is queued, still wins without reentrancy.
  QTimer::singleShot(0, this, [this]() {
    if (m_process && m_process->state() == QProcess::NotRunning)
      onProcessFinished(-1, QProcess::CrashExit);
  });
}
void LspClient::onProcessFinished(int code, QProcess::ExitStatus status) {
  if (!m_process)
    return;
  recordStderr(m_process->readAllStandardError());
  const bool expected = m_stopping;
  const bool wasInitialized = m_initialized;
  if (m_shutdownTimer)
    m_shutdownTimer->stop();
  if (!expected) {
    QString message =
        QString("LSP process exited (code %1, %2)")
            .arg(code)
            .arg(status == QProcess::CrashExit ? "crash" : "normal");
    if (!m_stderrPartial.trimmed().isEmpty())
      m_stderrLines.append(QString::fromUtf8(m_stderrPartial).trimmed());
    if (!m_stderrLines.isEmpty())
      message += "\n" + m_stderrLines.join("\n");
    emit serverError(message);
  }
  failPendingRequests("LSP server stopped");
  m_process->deleteLater();
  m_process = nullptr;
  resetSessionState();
  if (wasInitialized)
    emit serverStopped();
  emit processStopped(expected);
  if (m_startPending) {
    m_startPending = false;
    launch();
  }
}
