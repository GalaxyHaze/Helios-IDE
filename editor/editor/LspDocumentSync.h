#ifndef LSPDOCUMENTSYNC_H
#define LSPDOCUMENTSYNC_H

#include "LspTypes.h"

#include <QList>
#include <QString>

#include <optional>

struct LspDocumentSyncBatch
{
    int version = 0;
    QList<LspTextChange> changes;
    QString fullText;
    bool fullSync = false;
};

struct LspDocumentChangeInput
{
    int position = 0;
    int charsRemoved = 0;
    QString insertedText;
    QString currentText;
};

class LspDocumentSync
{
public:
    void setDocument(const QString &uri, const QString &text, int version);

    void recordChange(const LspDocumentChangeInput &change);

    std::optional<LspDocumentSyncBatch> takeBatch(int syncKind);
    void restoreBatch(const LspDocumentSyncBatch &batch);
    void discardPendingChanges();

    const QString &uri() const { return m_uri; }
    const QString &text() const { return m_text; }
    int version() const { return m_version; }

    static LspPosition positionForOffset(const QString &text, int offset);

private:
    QString m_uri;
    QString m_text;
    int m_version = 0;
    QList<LspTextChange> m_pendingChanges;
};

#endif
