#ifndef VIMPENDINGOPERATIONSESSION_H
#define VIMPENDINGOPERATIONSESSION_H

#include "VimCharacterSearch.h"
#include "VimDocumentOperations.h"
#include "VimOperation.h"

#include <QTextCursor>

class QKeyEvent;
class QPlainTextEdit;

struct VimPendingOperationResult
{
    bool handled = false;
    bool enterInsertMode = false;
};

struct VimPendingOperationStart
{
    VimPendingOperation operation = VimPendingOperation::None;
    int count = 1;
    int anchor = 0;
};

class VimPendingOperationSession
{
public:
    VimPendingOperationSession(QPlainTextEdit *editor,
                               VimCharacterSearch &characterSearch);

    void begin(const VimPendingOperationStart &start);
    VimPendingOperationResult handleKeyPress(QKeyEvent *event);
    void reset();

private:
    VimPendingOperationResult applySelection(const QTextCursor &selection);
    bool resolveMotion(QChar key, QTextCursor::MoveOperation &operation) const;

    QPlainTextEdit *m_editor = nullptr;
    VimDocumentOperations m_documentOperations;
    VimCharacterSearch &m_characterSearch;
    VimPendingOperation m_operation = VimPendingOperation::None;
    int m_count = 1;
    int m_anchor = 0;
    QChar m_pendingFind;
    bool m_findForward = true;
    bool m_findTill = false;
    bool m_waitingForG = false;
};

#endif
