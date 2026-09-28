#include "LspDocumentSync.h"

#include <QtGlobal>

void LspDocumentSync::setDocument(const QString &uri,
                                  const QString &text,
                                  int version)
{
    m_uri = uri;
    m_text = text;
    m_version = version;
    m_pendingChanges.clear();
}

void LspDocumentSync::recordChange(const LspDocumentChangeInput &change)
{
    const int documentLength = static_cast<int>(m_text.size());
    const int start = qBound(0, change.position, documentLength);
    const int removedEnd =
        qBound(0, change.position + change.charsRemoved, documentLength);

    m_pendingChanges.append(
        {{positionForOffset(m_text, start),
          positionForOffset(m_text, removedEnd)},
         change.insertedText});
    m_text = change.currentText;
}

std::optional<LspDocumentSyncBatch> LspDocumentSync::takeBatch(int syncKind)
{
    if (m_pendingChanges.isEmpty())
        return std::nullopt;

    ++m_version;
    LspDocumentSyncBatch batch;
    batch.version = m_version;
    batch.changes = m_pendingChanges;
    batch.fullText = m_text;
    batch.fullSync = syncKind != 2;
    m_pendingChanges.clear();
    return batch;
}

void LspDocumentSync::restoreBatch(const LspDocumentSyncBatch &batch)
{
    if (batch.changes.isEmpty())
        return;

    QList<LspTextChange> pending = batch.changes;
    pending.append(m_pendingChanges);
    m_pendingChanges = std::move(pending);
    if (m_version == batch.version)
        m_version = batch.version - 1;
}

void LspDocumentSync::discardPendingChanges()
{
    m_pendingChanges.clear();
}

LspPosition LspDocumentSync::positionForOffset(const QString &text,
                                               int offset)
{
    const int textLength = static_cast<int>(text.size());
    const int bounded = qBound(0, offset, textLength);
    int line = 0;
    int lineStart = 0;
    for (int index = 0; index < bounded; ++index) {
        if (text.at(index) == QLatin1Char('\n')) {
            ++line;
            lineStart = index + 1;
        }
    }
    return {line, static_cast<int>(bounded - lineStart)};
}
