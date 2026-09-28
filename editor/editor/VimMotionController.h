#ifndef VIMMOTIONCONTROLLER_H
#define VIMMOTIONCONTROLLER_H

#include "VimDocumentOperations.h"
#include "VimCharacterSearch.h"
#include "VimCommandSession.h"
#include "VimMotionResolver.h"
#include "VimPendingOperationSession.h"
#include "VimSearchSession.h"

#include <QObject>
#include <QTextCursor>
#include <QString>

class QKeyEvent;
class QPlainTextEdit;

class VimMotionController : public QObject
{
    Q_OBJECT

public:
    enum class Mode { Off, Normal, Insert };
    using PendingOp = VimPendingOperation;

    explicit VimMotionController(QPlainTextEdit *editor);
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    Mode mode() const { return m_enabled ? m_mode : Mode::Off; }
    bool handleKeyPress(QKeyEvent *event);

signals:
    void modeChanged(VimMotionController::Mode mode);
    void commandEntered(const QString &command);

private:
    void setMode(Mode mode);
    int takeCount();
    bool handleCommandMode(QKeyEvent *event);
    bool handlePendingOperation(QKeyEvent *event);
    bool handleVisualMotion(QKeyEvent *event);
    void resetPending();
    void resetCount();

    QPlainTextEdit *m_editor = nullptr;
    VimDocumentOperations m_documentOperations;
    VimCharacterSearch m_characterSearch;
    VimCommandSession m_commandSession;
    VimPendingOperationSession m_pendingOperationSession;
    VimSearchSession m_searchSession;
    bool m_enabled = false;
    Mode m_mode = Mode::Normal;
    int m_count = 0;
    QChar m_pendingFind;
    bool m_findForward = true;
    bool m_findTill = false;
    bool m_waitingForG = false;
    bool m_pendingReplace = false;
    bool m_visualMode = false;
    int m_visualAnchor = 0;
};

#endif
