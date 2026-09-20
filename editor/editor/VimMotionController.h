#ifndef VIMMOTIONCONTROLLER_H
#define VIMMOTIONCONTROLLER_H

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
    enum class PendingOp { None, Delete, Change, Yank };

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
    void move(QTextCursor::MoveOperation operation, int count);
    bool findCharacter(QChar character, bool forward, bool till, int count);
    bool repeatLastFind(bool reverseDirection, int count);
    void openLine(bool below);
    bool handleCommandMode(QKeyEvent *event);
    bool handleSearchMode(QKeyEvent *event);
    bool handlePendingOperation(QKeyEvent *event);
    bool handleVisualMotion(QKeyEvent *event);
    void startSearch(bool forward);
    void finishSearch();
    void repeatSearch(bool forward, int count);
    bool motionFromKey(QChar key, QTextCursor::MoveOperation &operation, int &count);
    void applyMotionToPendingOp(QTextCursor::MoveOperation operation, int count);
    void applySelectionToPendingOp(const QTextCursor &selection);
    void deleteLines(int count);
    void yankLines(int count);
    void changeLines(int count);
    void deleteVisualSelection();
    void yankVisualSelection();
    void changeVisualSelection();
    void resetPending();
    void resetCount();

    QPlainTextEdit *m_editor = nullptr;
    bool m_enabled = false;
    Mode m_mode = Mode::Normal;
    int m_count = 0;
    QChar m_pendingFind;
    QChar m_lastFindCharacter;
    bool m_findForward = true;
    bool m_findTill = false;
    bool m_lastFindForward = true;
    bool m_lastFindTill = false;
    bool m_waitingForG = false;
    PendingOp m_pendingOp = PendingOp::None;
    int m_pendingOpCount = 1;
    int m_pendingOpAnchor = 0;
    bool m_pendingReplace = false;
    bool m_visualMode = false;
    int m_visualAnchor = 0;
    bool m_commandMode = false;
    bool m_searchMode = false;
    QString m_commandLine;
    QString m_searchInput;
    QString m_lastSearch;
    bool m_lastSearchForward = true;
};

#endif
