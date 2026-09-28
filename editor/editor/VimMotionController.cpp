#include "VimMotionController.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

VimMotionController::VimMotionController(QPlainTextEdit *editor)
    : QObject(editor), m_editor(editor), m_documentOperations(editor),
      m_characterSearch(editor),
      m_pendingOperationSession(editor, m_characterSearch),
      m_searchSession(editor) {}

void VimMotionController::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    m_count = 0;
    m_characterSearch.reset();
    resetPending();
    setMode(enabled ? Mode::Normal : Mode::Off);
}

void VimMotionController::setMode(Mode mode)
{
    if (m_mode == mode && (mode != Mode::Off || !m_enabled))
        return;
    m_mode = mode;
    emit modeChanged(this->mode());
}

int VimMotionController::takeCount()
{
    const int count = m_count == 0 ? 1 : m_count;
    m_count = 0;
    return count;
}

bool VimMotionController::handleCommandMode(QKeyEvent *event)
{
    if (!m_commandSession.isActive())
        return false;

    m_commandSession.handleKeyPress(event);
    const std::optional<QString> command =
        m_commandSession.takeSubmittedCommand();
    if (command.has_value())
        emit commandEntered(*command);
    return true;
}

bool VimMotionController::handlePendingOperation(QKeyEvent *event)
{
    const VimPendingOperationResult result =
        m_pendingOperationSession.handleKeyPress(event);
    if (!result.handled)
        return false;
    if (result.enterInsertMode)
        setMode(Mode::Insert);
    return true;
}

bool VimMotionController::handleVisualMotion(QKeyEvent *event)
{
    if (!m_visualMode)
        return false;

    const QString text = event->text();
    if (text.isEmpty())
        return true;
    const QChar key = text.at(0);
    if (key == QLatin1Char('v')) {
        m_visualMode = false;
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_visualAnchor);
        m_editor->setTextCursor(cursor);
        return true;
    }
    if (key == QLatin1Char('d') || key == QLatin1Char('x')) {
        m_documentOperations.deleteSelection();
        m_visualMode = false;
        return true;
    }
    if (key == QLatin1Char('y')) {
        m_documentOperations.yankSelection();
        m_visualMode = false;
        return true;
    }
    if (key == QLatin1Char('c')) {
        m_documentOperations.deleteSelection();
        m_visualMode = false;
        setMode(Mode::Insert);
        return true;
    }

    std::optional<VimMotion> motion = VimMotionResolver::resolve(key);
    if (!motion.has_value() && key == QLatin1Char('0'))
        motion = VimMotion{QTextCursor::StartOfBlock};
    if (!motion.has_value() && key == QLatin1Char('$'))
        motion = VimMotion{QTextCursor::EndOfBlock};
    if (motion.has_value()) {
        m_documentOperations.move(motion->operation, 1);
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_visualAnchor, QTextCursor::KeepAnchor);
        m_editor->setTextCursor(cursor);
        return true;
    }
    return true;
}

void VimMotionController::resetCount()
{
    m_count = 0;
    m_pendingFind = {};
    m_waitingForG = false;
}

void VimMotionController::resetPending()
{
    m_pendingOperationSession.reset();
    m_pendingReplace = false;
    m_visualMode = false;
    m_commandSession.reset();
    m_searchSession.reset();
}

bool VimMotionController::handleKeyPress(QKeyEvent *event)
{
    if (!m_enabled || !m_editor)
        return false;

    if (m_mode == Mode::Insert) {
        if (event->key() == Qt::Key_Escape) {
            setMode(Mode::Normal);
            return true;
        }
        return false;
    }

    if (handleCommandMode(event))
        return true;
    if (m_searchSession.handleKeyPress(event))
        return true;
    if (handlePendingOperation(event))
        return true;
    if (handleVisualMotion(event))
        return true;

    if (event->key() == Qt::Key_Escape) {
        resetCount();
        resetPending();
        return true;
    }
    if (event->modifiers() == Qt::ControlModifier &&
        event->key() == Qt::Key_R) {
        m_editor->redo();
        return true;
    }
    if (event->modifiers() != Qt::NoModifier)
        return false;

    const QString text = event->text();
    if (text.isEmpty())
        return false;
    const QChar key = text.at(0);

    if (key == QLatin1Char('/')) {
        m_searchSession.begin(true);
        return true;
    }
    if (key == QLatin1Char('?')) {
        m_searchSession.begin(false);
        return true;
    }
    if (key == QLatin1Char('n') || key == QLatin1Char('N')) {
        m_searchSession.repeat(key == QLatin1Char('n'), takeCount());
        return true;
    }
    if (key == QLatin1Char(':')) {
        m_commandSession.begin();
        return true;
    }
    if (key == QLatin1Char('v')) {
        m_visualMode = true;
        m_visualAnchor = m_editor->textCursor().position();
        return true;
    }
    if (key == QLatin1Char('d') || key == QLatin1Char('y') ||
        key == QLatin1Char('c')) {
        const PendingOp operation =
            key == QLatin1Char('d')
                ? PendingOp::Delete
                : (key == QLatin1Char('y') ? PendingOp::Yank
                                           : PendingOp::Change);
        m_pendingOperationSession.begin(
            {operation, m_count == 0 ? 1 : m_count,
             m_editor->textCursor().position()});
        m_count = 0;
        return true;
    }
    if (key == QLatin1Char('x')) {
        m_documentOperations.deleteCharacters(takeCount(), false);
        return true;
    }
    if (key == QLatin1Char('X')) {
        m_documentOperations.deleteCharacters(takeCount(), true);
        return true;
    }
    if (key == QLatin1Char('u')) {
        m_editor->undo();
        return true;
    }
    if (key == QLatin1Char('p') || key == QLatin1Char('P')) {
        m_documentOperations.paste(key == QLatin1Char('p'));
        return true;
    }
    if (m_pendingReplace) {
        m_pendingReplace = false;
        m_documentOperations.replaceCharacters(text, takeCount());
        return true;
    }
    if (key == QLatin1Char('r')) {
        m_pendingReplace = true;
        return true;
    }
    if (!m_pendingFind.isNull()) {
        const int count = takeCount();
        const bool result = m_characterSearch.find(
            {key, m_findForward, m_findTill, count});
        m_pendingFind = {};
        return result;
    }
    if (m_waitingForG) {
        m_waitingForG = false;
        if (key == QLatin1Char('g')) {
            m_documentOperations.move(QTextCursor::Start, takeCount());
            return true;
        }
        return key == QLatin1Char('G');
    }
    if (key.isDigit() && (key != QLatin1Char('0') || m_count > 0)) {
        m_count = qMin(9999, m_count * 10 + key.digitValue());
        return true;
    }

    const int count = takeCount();
    const std::optional<VimMotion> motion = VimMotionResolver::resolve(key);
    if (motion.has_value()) {
        m_documentOperations.move(motion->operation, count);
        return true;
    }

    switch (key.unicode()) {
    case '0':
        m_documentOperations.move(QTextCursor::StartOfBlock, 1);
        return true;
    case '^': {
        QTextCursor cursor = m_editor->textCursor();
        const QString line = cursor.block().text();
        cursor.setPosition(cursor.block().position() +
                           line.indexOf(QRegularExpression("\\S")));
        m_editor->setTextCursor(cursor);
        return true;
    }
    case '$':
        m_documentOperations.move(QTextCursor::EndOfBlock, 1);
        return true;
    case 'g':
        m_waitingForG = true;
        m_count = count == 1 ? 0 : count;
        return true;
    case 'G':
        m_documentOperations.move(QTextCursor::End, 1);
        return true;
    case 'f':
        m_pendingFind = key;
        m_findForward = true;
        m_findTill = false;
        m_count = count == 1 ? 0 : count;
        return true;
    case 'F':
        m_pendingFind = key;
        m_findForward = false;
        m_findTill = false;
        m_count = count == 1 ? 0 : count;
        return true;
    case 't':
        m_pendingFind = key;
        m_findForward = true;
        m_findTill = true;
        m_count = count == 1 ? 0 : count;
        return true;
    case 'T':
        m_pendingFind = key;
        m_findForward = false;
        m_findTill = true;
        m_count = count == 1 ? 0 : count;
        return true;
    case ';':
        return m_characterSearch.repeatLast(false, count);
    case ',':
        return m_characterSearch.repeatLast(true, count);
    case 'I':
        m_documentOperations.move(QTextCursor::StartOfBlock, 1);
        setMode(Mode::Insert);
        return true;
    case 'A':
        m_documentOperations.move(QTextCursor::EndOfBlock, 1);
        setMode(Mode::Insert);
        return true;
    case 'o':
        m_documentOperations.openLine(true);
        setMode(Mode::Insert);
        return true;
    case 'O':
        m_documentOperations.openLine(false);
        setMode(Mode::Insert);
        return true;
    case 'i':
        setMode(Mode::Insert);
        return true;
    case 'a':
        m_documentOperations.move(QTextCursor::Right, 1);
        setMode(Mode::Insert);
        return true;
    default:
        return true;
    }
}
