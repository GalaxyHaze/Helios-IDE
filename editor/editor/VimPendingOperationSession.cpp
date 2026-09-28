#include "VimPendingOperationSession.h"

#include "VimMotionResolver.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QTextDocument>

VimPendingOperationSession::VimPendingOperationSession(
    QPlainTextEdit *editor, VimCharacterSearch &characterSearch)
    : m_editor(editor), m_documentOperations(editor),
      m_characterSearch(characterSearch)
{
}

void VimPendingOperationSession::begin(
    const VimPendingOperationStart &start)
{
    m_operation = start.operation;
    m_count = qMax(1, start.count);
    m_anchor = start.anchor;
    m_pendingFind = {};
    m_findForward = true;
    m_findTill = false;
    m_waitingForG = false;
}

VimPendingOperationResult VimPendingOperationSession::applySelection(
    const QTextCursor &selection)
{
    const bool enterInsertMode =
        m_documentOperations.applySelection(selection, m_operation);
    reset();
    return {true, enterInsertMode};
}

bool VimPendingOperationSession::resolveMotion(
    QChar key, QTextCursor::MoveOperation &operation) const
{
    const std::optional<VimMotion> motion = VimMotionResolver::resolve(key);
    if (motion.has_value()) {
        operation = motion->operation;
        return true;
    }
    if (key == QLatin1Char('0')) {
        operation = QTextCursor::StartOfBlock;
        return true;
    }
    if (key == QLatin1Char('$')) {
        operation = QTextCursor::EndOfBlock;
        return true;
    }
    return false;
}

VimPendingOperationResult VimPendingOperationSession::handleKeyPress(
    QKeyEvent *event)
{
    if (m_operation == VimPendingOperation::None || !event || !m_editor)
        return {};

    const QString text = event->text();
    const QChar key = text.isEmpty() ? QChar() : text.at(0);
    if (key == QLatin1Char('d') &&
        m_operation == VimPendingOperation::Delete) {
        m_documentOperations.deleteLines(m_count);
        reset();
        return {true, false};
    }
    if (key == QLatin1Char('y') &&
        m_operation == VimPendingOperation::Yank) {
        m_documentOperations.yankLines(m_count);
        reset();
        return {true, false};
    }
    if (key == QLatin1Char('c') &&
        m_operation == VimPendingOperation::Change) {
        m_documentOperations.changeLines(m_count);
        reset();
        return {true, true};
    }
    if (key == QLatin1Char('g') && !m_waitingForG) {
        m_waitingForG = true;
        return {true, false};
    }
    if (key == QLatin1Char('f') || key == QLatin1Char('F') ||
        key == QLatin1Char('t') || key == QLatin1Char('T')) {
        m_pendingFind = key;
        m_findForward =
            key == QLatin1Char('f') || key == QLatin1Char('t');
        m_findTill = key == QLatin1Char('t') || key == QLatin1Char('T');
        return {true, false};
    }
    if (!m_pendingFind.isNull()) {
        const bool found = m_characterSearch.find(
            {key, m_findForward, m_findTill, m_count});
        m_pendingFind = {};
        if (!found) {
            reset();
            return {true, false};
        }

        QTextCursor anchor(m_editor->document());
        anchor.setPosition(m_anchor);
        QTextCursor target = m_editor->textCursor();
        target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
        return applySelection(target);
    }

    QTextCursor::MoveOperation operation;
    if (resolveMotion(key, operation)) {
        m_waitingForG = false;
        QTextCursor cursor = m_editor->textCursor();
        cursor.movePosition(operation, QTextCursor::MoveAnchor, 1);
        m_editor->setTextCursor(cursor);

        QTextCursor anchor(m_editor->document());
        anchor.setPosition(m_anchor);
        QTextCursor target = m_editor->textCursor();
        target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
        return applySelection(target);
    }

    if (m_waitingForG) {
        m_waitingForG = false;
        if (key == QLatin1Char('g')) {
            QTextCursor anchor(m_editor->document());
            anchor.setPosition(m_anchor);
            QTextCursor target(m_editor->document());
            target.setPosition(0);
            target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
            return applySelection(target);
        }
        if (key == QLatin1Char('G')) {
            QTextCursor anchor(m_editor->document());
            anchor.setPosition(m_anchor);
            QTextCursor target(m_editor->document());
            target.movePosition(QTextCursor::End);
            target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
            return applySelection(target);
        }
    }

    reset();
    return {true, false};
}

void VimPendingOperationSession::reset()
{
    m_operation = VimPendingOperation::None;
    m_count = 1;
    m_anchor = 0;
    m_pendingFind = {};
    m_findForward = true;
    m_findTill = false;
    m_waitingForG = false;
}
