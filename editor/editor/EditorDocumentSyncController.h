#ifndef EDITORDOCUMENTSYNCCONTROLLER_H
#define EDITORDOCUMENTSYNCCONTROLLER_H

#include "LspDocumentSync.h"

#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>

struct EditorDocumentSyncCallbacks
{
    std::function<bool()> canSend;
    std::function<int()> syncKind;
    std::function<void(const QString &, const QList<LspTextChange> &, int)>
        sendChanges;
    std::function<void(const QString &, const QString &, int)> sendFullText;
};

class EditorDocumentSyncController : public QObject
{
    Q_OBJECT

public:
    explicit EditorDocumentSyncController(
        EditorDocumentSyncCallbacks callbacks, QObject *parent = nullptr);

    void setDocument(const QString &uri, const QString &text, int version);
    void recordChange(const LspDocumentChangeInput &change);
    void flush();
    void markDocumentSynchronized();
    void discardPendingChanges();
    void stop();

    int version() const;

private:
    EditorDocumentSyncCallbacks m_callbacks;
    LspDocumentSync m_documentSync;
    QTimer m_flushTimer;
};

#endif
