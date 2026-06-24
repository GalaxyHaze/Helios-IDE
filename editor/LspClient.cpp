#include "LspClient.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

LspClient::LspClient(QObject *parent)
    : QObject(parent)
{
}

LspClient::~LspClient()
{
    stop();
}

bool LspClient::start(const QString &serverPath, const QString &stdlibPath)
{
    if (m_process)
        stop();

    if (!QFileInfo::exists(serverPath)) {
        emit serverError("LSP server not found: " + serverPath);
        return false;
    }

    m_process = new QProcess(this);
    m_process->setProgram(serverPath);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    QObject::connect(m_process, &QProcess::readyReadStandardOutput,
                     this, &LspClient::onReadyRead);
    QObject::connect(m_process, &QProcess::readyReadStandardError,
                     this, [this]() {
        QByteArray err = m_process->readAllStandardError();
        if (!err.isEmpty())
            qDebug().noquote() << "[zith-lsp]" << err.trimmed();
    });
    QObject::connect(m_process, &QProcess::errorOccurred,
                     this, &LspClient::onProcessError);
    QObject::connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &LspClient::onProcessFinished);

    m_process->start(QIODevice::ReadWrite);
    if (!m_process->waitForStarted(5000)) {
        emit serverError("Failed to start LSP server: " + m_process->errorString());
        delete m_process;
        m_process = nullptr;
        return false;
    }

    QJsonObject clientCapabilities;
    clientCapabilities["textDocument"] = QJsonObject{
        {"completion", QJsonObject{
            {"completionItem", QJsonObject{
                {"snippetSupport", false}
            }}
        }},
        {"hover", QJsonObject{
            {"contentFormat", QJsonArray{"markdown", "plaintext"}}
        }},
        {"definition", QJsonObject{}}
    };

    QJsonObject initParams;
    initParams["processId"] = QJsonValue::Null;
    initParams["capabilities"] = clientCapabilities;
    initParams["rootUri"] = QUrl::fromLocalFile(QDir::currentPath()).toString();
    initParams["rootPath"] = QDir::currentPath();

    if (!stdlibPath.isEmpty()) {
        QJsonObject initOpts;
        initOpts["stdlibPath"] = stdlibPath;
        initParams["initializationOptions"] = initOpts;
    }

    sendRequest("initialize", initParams, [this](const QJsonObject &resp) {
        m_initialized = true;
        emit initialized();

        QJsonObject initNotify;
        initNotify["jsonrpc"] = "2.0";
        initNotify["method"] = "initialized";
        initNotify["params"] = QJsonObject();
        sendMessage(initNotify);
    });

    return true;
}

void LspClient::stop()
{
    if (!m_process)
        return;

    if (!m_shutdownRequested) {
        m_shutdownRequested = true;

        QJsonObject shutdown;
        shutdown["jsonrpc"] = "2.0";
        shutdown["id"] = nextId();
        shutdown["method"] = "shutdown";
        sendMessage(shutdown);
    }

    QJsonObject exit;
    exit["jsonrpc"] = "2.0";
    exit["method"] = "exit";
    sendMessage(exit);

    if (!m_process->waitForFinished(2000)) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }

    m_process->deleteLater();
    m_process = nullptr;
    m_initialized = false;
    m_pendingRequests.clear();
    m_buffer.clear();
}

bool LspClient::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void LspClient::openDocument(const QString &uri, const QString &languageId, const QString &text)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;
    textDoc["languageId"] = languageId;
    textDoc["version"] = 1;
    textDoc["text"] = text;

    QJsonObject params;
    params["textDocument"] = textDoc;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didOpen";
    msg["params"] = params;
    sendMessage(msg);
}

void LspClient::changeDocument(const QString &uri, const QString &text, int version)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;
    textDoc["version"] = version;

    QJsonObject contentChange;
    contentChange["text"] = text;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["contentChanges"] = QJsonArray{contentChange};

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didChange";
    msg["params"] = params;
    sendMessage(msg);
}

void LspClient::closeDocument(const QString &uri)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject params;
    params["textDocument"] = textDoc;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didClose";
    msg["params"] = params;
    sendMessage(msg);
}

void LspClient::saveDocument(const QString &uri)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject params;
    params["textDocument"] = textDoc;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didSave";
    msg["params"] = params;
    sendMessage(msg);
}

void LspClient::requestCompletion(const QString &uri, const LspPosition &pos)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject position;
    position["line"] = pos.line;
    position["character"] = pos.character;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["position"] = position;

    sendRequest("textDocument/completion", params, [this](const QJsonObject &resp) {
        QList<LspCompletionItem> items;
        QJsonValue result = resp["result"];
        QJsonArray arr;

        if (result.isObject()) {
            arr = result.toObject()["items"].toArray();
        } else if (result.isArray()) {
            arr = result.toArray();
        }

        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            LspCompletionItem item;
            item.label = obj["label"].toString();
            item.kind = obj["kind"].toInt();
            item.detail = obj["detail"].toString();
            item.insertText = obj["insertText"].toString();
            if (item.insertText.isEmpty())
                item.insertText = item.label;
            item.insertTextFormat = obj["insertTextFormat"].toInt(1);
            items.append(item);
        }

        emit completionResults(items);
    });
}

void LspClient::requestHover(const QString &uri, const LspPosition &pos)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject position;
    position["line"] = pos.line;
    position["character"] = pos.character;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["position"] = position;

    sendRequest("textDocument/hover", params, [this](const QJsonObject &resp) {
        QJsonObject result = resp["result"].toObject();
        if (result.isEmpty()) {
            emit hoverResult({});
            return;
        }

        LspHoverInfo info;

        QJsonValue contents = result["contents"];
        if (contents.isString()) {
            info.contents = contents.toString();
        } else if (contents.isObject()) {
            info.contents = contents.toObject()["value"].toString();
        } else if (contents.isArray()) {
            QStringList parts;
            for (const QJsonValue &v : contents.toArray()) {
                if (v.isString())
                    parts.append(v.toString());
                else if (v.isObject())
                    parts.append(v.toObject()["value"].toString());
            }
            info.contents = parts.join("\n\n");
        }

        QJsonObject range = result["range"].toObject();
        if (!range.isEmpty()) {
            info.range.start.line = range["start"].toObject()["line"].toInt();
            info.range.start.character = range["start"].toObject()["character"].toInt();
            info.range.end.line = range["end"].toObject()["line"].toInt();
            info.range.end.character = range["end"].toObject()["character"].toInt();
        }

        emit hoverResult(info);
    });
}

void LspClient::requestDefinition(const QString &uri, const LspPosition &pos)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject position;
    position["line"] = pos.line;
    position["character"] = pos.character;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["position"] = position;

    sendRequest("textDocument/definition", params, [this](const QJsonObject &resp) {
        QJsonValue result = resp["result"];
        QJsonObject loc;

        if (result.isArray()) {
            QJsonArray arr = result.toArray();
            if (!arr.isEmpty())
                loc = arr.first().toObject();
        } else if (result.isObject()) {
            loc = result.toObject();
        }

        if (loc.isEmpty()) {
            emit definitionResult({});
            return;
        }

        LspLocation location;
        location.uri = loc["uri"].toString();
        QJsonObject r = loc["range"].toObject();
        location.range.start.line = r["start"].toObject()["line"].toInt();
        location.range.start.character = r["start"].toObject()["character"].toInt();
        location.range.end.line = r["end"].toObject()["line"].toInt();
        location.range.end.character = r["end"].toObject()["character"].toInt();

        emit definitionResult(location);
    });
}

void LspClient::requestSignatureHelp(const QString &uri, const LspPosition &pos)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject position;
    position["line"] = pos.line;
    position["character"] = pos.character;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["position"] = position;

    sendRequest("textDocument/signatureHelp", params, [this](const QJsonObject &resp) {
        QJsonObject result = resp["result"].toObject();
        if (result.isEmpty()) {
            emit signatureHelpResult({});
            return;
        }

        LspSignatureHelp help;
        QJsonArray sigs = result["signatures"].toArray();
        if (!sigs.isEmpty()) {
            QJsonObject sig = sigs.first().toObject();
            help.activeSignature = sig["label"].toString();
            QJsonArray params = sig["parameters"].toArray();
            for (const QJsonValue &p : params)
                help.parameters.append(p.toObject()["label"].toString());
        }
        help.activeParameter = result["activeParameter"].toInt();

        emit signatureHelpResult(help);
    });
}

void LspClient::requestSemanticTokens(const QString &uri)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject params;
    params["textDocument"] = textDoc;

    sendRequest("textDocument/semanticTokens/full", params, [this](const QJsonObject &resp) {
        QJsonArray tokens = resp["result"].toObject()["data"].toArray();
        emit semanticTokensResult(tokens);
    });
}

void LspClient::requestFormatting(const QString &uri)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject options;
    options["tabSize"] = 4;
    options["insertSpaces"] = true;

    QJsonObject params;
    params["textDocument"] = textDoc;
    params["options"] = options;

    sendRequest("textDocument/formatting", params, [this](const QJsonObject &resp) {
        QList<QPair<LspRange, QString>> edits;
        QJsonArray arr = resp["result"].toArray();
        for (const QJsonValue &val : arr) {
            QJsonObject edit = val.toObject();
            QJsonObject r = edit["range"].toObject();
            LspRange range;
            range.start.line = r["start"].toObject()["line"].toInt();
            range.start.character = r["start"].toObject()["character"].toInt();
            range.end.line = r["end"].toObject()["line"].toInt();
            range.end.character = r["end"].toObject()["character"].toInt();
            edits.append({range, edit["newText"].toString()});
        }
        emit formattingResult(edits);
    });
}

void LspClient::requestDocumentSymbols(const QString &uri)
{
    QJsonObject textDoc;
    textDoc["uri"] = uri;

    QJsonObject params;
    params["textDocument"] = textDoc;

    sendRequest("textDocument/documentSymbol", params, [this](const QJsonObject &resp) {
        QJsonArray symbols = resp["result"].toArray();
        emit documentSymbolsResult(symbols);
    });
}

// ── Private ──────────────────────────────────────────────────────────

int LspClient::nextId()
{
    return ++m_nextId;
}

void LspClient::sendMessage(const QJsonObject &msg)
{
    if (!m_process || m_process->state() != QProcess::Running)
        return;

    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    QByteArray header = "Content-Length: " + QByteArray::number(data.size()) + "\r\n\r\n";
    m_process->write(header + data);
}

void LspClient::sendRequest(const QString &method, const QJsonObject &params,
                            std::function<void(const QJsonObject &)> callback)
{
    int id = nextId();
    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["id"] = id;
    msg["method"] = method;
    msg["params"] = params;

    if (callback)
        m_pendingRequests.insert(id, {callback});

    sendMessage(msg);
}

void LspClient::onReadyRead()
{
    m_buffer.append(m_process->readAllStandardOutput());
    processBuffer();
}

void LspClient::processBuffer()
{
    while (true) {
        int headerEnd = m_buffer.indexOf("\r\n\r\n");
        if (headerEnd == -1)
            return;

        QByteArray header = m_buffer.left(headerEnd);
        int contentLength = 0;
        for (const QByteArray &line : header.split('\n')) {
            QByteArray trimmed = line.trimmed();
            if (trimmed.startsWith("Content-Length: "))
                contentLength = trimmed.mid(16).toInt();
        }

        if (contentLength <= 0) {
            m_buffer.remove(0, headerEnd + 4);
            continue;
        }

        int bodyStart = headerEnd + 4;
        if (m_buffer.size() < bodyStart + contentLength)
            return;

        QByteArray body = m_buffer.mid(bodyStart, contentLength);
        m_buffer.remove(0, bodyStart + contentLength);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "LSP JSON parse error:" << parseError.errorString();
            continue;
        }

        QJsonObject msg = doc.object();
        if (msg.contains("id") && (msg.contains("result") || msg.contains("error")))
            handleResponse(msg);
        else if (msg.contains("method"))
            handleNotification(msg);
    }
}

void LspClient::handleResponse(const QJsonObject &msg)
{
    QJsonValue idVal = msg["id"];
    int id = -1;
    if (idVal.isDouble())
        id = idVal.toInt();
    else if (idVal.isString())
        id = idVal.toString().toInt();

    if (id >= 0 && m_pendingRequests.contains(id)) {
        auto req = m_pendingRequests.take(id);
        if (req.callback)
            req.callback(msg);
    }

    if (msg.contains("error") && !msg["error"].isNull()) {
        QJsonObject err = msg["error"].toObject();
        qWarning() << "LSP error:" << err["code"].toInt() << err["message"].toString();
    }
}

void LspClient::handleNotification(const QJsonObject &msg)
{
    QString method = msg["method"].toString();
    QJsonObject params = msg["params"].toObject();

    if (method == "textDocument/publishDiagnostics") {
        QString uri = params["uri"].toString();
        QList<LspDiagnostic> diags;

        for (const QJsonValue &val : params["diagnostics"].toArray()) {
            QJsonObject d = val.toObject();
            LspDiagnostic diag;
            QJsonObject r = d["range"].toObject();
            diag.range.start.line = r["start"].toObject()["line"].toInt();
            diag.range.start.character = r["start"].toObject()["character"].toInt();
            diag.range.end.line = r["end"].toObject()["line"].toInt();
            diag.range.end.character = r["end"].toObject()["character"].toInt();
            diag.severity = d["severity"].toInt();
            diag.message = d["message"].toString();
            diag.source = d["source"].toString();
            diags.append(diag);
        }

        emit diagnosticsReceived(uri, diags);
    } else if (method == "window/logMessage") {
        emit logMessage(params["message"].toString());
    }
}

void LspClient::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    emit serverError("LSP process error: " + (m_process ? m_process->errorString() : "unknown"));
}

void LspClient::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::CrashExit)
        emit serverError("LSP server crashed with exit code " + QString::number(exitCode));
}
