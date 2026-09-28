#include "EditorDocumentSyncController.h"

#include <utility>

EditorDocumentSyncController::EditorDocumentSyncController(
    EditorDocumentSyncCallbacks callbacks, QObject *parent)
    : QObject(parent), m_callbacks(std::move(callbacks))
{
    m_flushTimer.setSingleShot(true);
    m_flushTimer.setInterval(40);
    connect(&m_flushTimer, &QTimer::timeout, this,
            &EditorDocumentSyncController::flush);
}

void EditorDocumentSyncController::setDocument(const QString &uri,
                                                const QString &text,
                                                int version)
{
    m_flushTimer.stop();
    m_documentSync.setDocument(uri, text, version);
}

void EditorDocumentSyncController::recordChange(
    const LspDocumentChangeInput &change)
{
    if (m_documentSync.uri().isEmpty())
        return;

    m_documentSync.recordChange(change);
    m_flushTimer.start();
}

void EditorDocumentSyncController::flush()
{
    m_flushTimer.stop();
    if (m_documentSync.uri().isEmpty() || !m_callbacks.canSend ||
        !m_callbacks.canSend()) {
        return;
    }

    const int syncKind = m_callbacks.syncKind ? m_callbacks.syncKind() : 1;
    const bool fullSync = syncKind != 2;
    if ((fullSync && !m_callbacks.sendFullText) ||
        (!fullSync && !m_callbacks.sendChanges)) {
        return;
    }

    const auto batch = m_documentSync.takeBatch(syncKind);
    if (!batch.has_value())
        return;

    if (!batch->fullSync) {
        m_callbacks.sendChanges(m_documentSync.uri(), batch->changes,
                                batch->version);
    } else {
        m_callbacks.sendFullText(m_documentSync.uri(), batch->fullText,
                                 batch->version);
    }
}

void EditorDocumentSyncController::markDocumentSynchronized()
{
    m_flushTimer.stop();
    m_documentSync.discardPendingChanges();
}

void EditorDocumentSyncController::discardPendingChanges()
{
    m_flushTimer.stop();
    m_documentSync.discardPendingChanges();
}

void EditorDocumentSyncController::stop()
{
    m_flushTimer.stop();
}

int EditorDocumentSyncController::version() const
{
    return m_documentSync.version();
}
