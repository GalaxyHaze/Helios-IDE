#include "LspDocumentProtocol.h"

#include <QJsonArray>

#include <utility>

namespace {
QJsonObject positionObject(const LspPosition &position)
{
    return {{"line", position.line}, {"character", position.character}};
}

QJsonObject rangeObject(const LspRange &range)
{
    return {{"start", positionObject(range.start)},
            {"end", positionObject(range.end)}};
}

bool isReady(const LspDocumentProtocolCallbacks &callbacks)
{
    return callbacks.isReady && callbacks.isReady();
}

bool sendMessage(const LspDocumentProtocolCallbacks &callbacks,
                 const QJsonObject &message)
{
    return callbacks.sendMessage && callbacks.sendMessage(message);
}
} // namespace

LspDocumentProtocol::LspDocumentProtocol(
    LspDocumentProtocolCallbacks callbacks)
    : m_callbacks(std::move(callbacks))
{
}

void LspDocumentProtocol::clear()
{
    m_documentRegistry.clear();
}

int LspDocumentProtocol::documentVersion(const QString &uri) const
{
    return m_documentRegistry.version(uri);
}

bool LspDocumentProtocol::isCurrentDocument(const QString &uri,
                                            int version) const
{
    return m_documentRegistry.isCurrent(uri, version);
}

void LspDocumentProtocol::openDocument(const QString &uri,
                                       const QString &languageId,
                                       const QString &text,
                                       int version)
{
    if (!isReady(m_callbacks))
        return;

    m_documentRegistry.open(uri, version);
    sendMessage(
        m_callbacks,
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didOpen"},
         {"params",
          QJsonObject{
              {"textDocument",
               QJsonObject{{"uri", uri},
                           {"languageId", languageId},
                           {"version", version},
                           {"text", text}}}}}});
}

bool LspDocumentProtocol::changeDocument(
    const QString &uri, const QList<LspTextChange> &changes, int version)
{
    if (!isReady(m_callbacks) || changes.isEmpty())
        return false;

    if (m_callbacks.cancelRequestsForUri)
        m_callbacks.cancelRequestsForUri(uri);

    QJsonArray content;
    for (const LspTextChange &change : changes) {
        content.append(QJsonObject{{"range", rangeObject(change.range)},
                                   {"text", change.text}});
    }
    const bool sent = sendMessage(
        m_callbacks,
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didChange"},
         {"params",
          QJsonObject{
              {"textDocument",
               QJsonObject{{"uri", uri}, {"version", version}}},
              {"contentChanges", content}}}});
    if (!sent)
        return false;

    m_documentRegistry.update(uri, version);
    return true;
}

bool LspDocumentProtocol::changeDocumentFull(const QString &uri,
                                              const QString &fullText,
                                              int version)
{
    if (!isReady(m_callbacks))
        return false;

    if (m_callbacks.cancelRequestsForUri)
        m_callbacks.cancelRequestsForUri(uri);

    const bool sent = sendMessage(
        m_callbacks,
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didChange"},
         {"params",
          QJsonObject{
              {"textDocument",
               QJsonObject{{"uri", uri}, {"version", version}}},
              {"contentChanges", QJsonArray{QJsonObject{{"text", fullText}}}}}}});
    if (!sent)
        return false;

    m_documentRegistry.update(uri, version);
    return true;
}

void LspDocumentProtocol::closeDocument(const QString &uri)
{
    if (m_callbacks.cancelRequestsForUri)
        m_callbacks.cancelRequestsForUri(uri);
    m_documentRegistry.close(uri);

    if (!isReady(m_callbacks))
        return;

    sendMessage(
        m_callbacks,
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didClose"},
         {"params",
          QJsonObject{{"textDocument", QJsonObject{{"uri", uri}}}}}});
}

void LspDocumentProtocol::saveDocument(const QString &uri)
{
    if (!isReady(m_callbacks))
        return;

    sendMessage(
        m_callbacks,
        {{"jsonrpc", "2.0"},
         {"method", "textDocument/didSave"},
         {"params",
          QJsonObject{{"textDocument", QJsonObject{{"uri", uri}}}}}});
}
