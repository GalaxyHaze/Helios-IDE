#include "VimSearchSession.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>

VimSearchSession::VimSearchSession(QPlainTextEdit *editor)
    : m_editor(editor)
{
}

void VimSearchSession::begin(bool forward)
{
    m_active = true;
    m_lastSearchForward = forward;
    m_input.clear();
}

bool VimSearchSession::handleKeyPress(QKeyEvent *event)
{
    if (!m_active)
        return false;

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Escape) {
        finish();
        return true;
    }
    if (event->key() == Qt::Key_Backspace) {
        m_input.chop(1);
        return true;
    }

    const QString text = event->text();
    if (!text.isEmpty())
        m_input += text;
    return true;
}

void VimSearchSession::finish()
{
    m_active = false;
    if (m_input.isEmpty()) {
        m_input.clear();
        return;
    }

    m_lastSearch = m_input;
    m_input.clear();
    find(m_lastSearchForward, 1);
}

void VimSearchSession::repeat(bool forward, int count)
{
    find(forward, count);
}

void VimSearchSession::find(bool forward, int count)
{
    if (!m_editor || m_lastSearch.isEmpty())
        return;

    QTextCursor cursor = m_editor->textCursor();
    const QTextDocument::FindFlags flags =
        forward ? QTextDocument::FindCaseSensitively
                : QTextDocument::FindBackward |
                      QTextDocument::FindCaseSensitively;
    for (int i = 0; i < count; ++i) {
        cursor = m_editor->document()->find(m_lastSearch, cursor, flags);
        if (cursor.isNull()) {
            cursor = m_editor->document()->find(
                m_lastSearch,
                forward ? 0 : m_editor->document()->characterCount(), flags);
        }
    }
    if (!cursor.isNull())
        m_editor->setTextCursor(cursor);
}

void VimSearchSession::reset()
{
    m_active = false;
    m_input.clear();
}
