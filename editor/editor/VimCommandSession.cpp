#include "VimCommandSession.h"

#include <QKeyEvent>

void VimCommandSession::begin()
{
    m_active = true;
    m_input.clear();
    m_submittedCommand.reset();
}

bool VimCommandSession::handleKeyPress(QKeyEvent *event)
{
    if (!m_active || event == nullptr) {
        return false;
    }

    if (event->key() == Qt::Key_Escape) {
        reset();
        return true;
    }

    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        const QString command = m_input.trimmed();
        m_input.clear();
        m_active = false;
        if (!command.isEmpty()) {
            m_submittedCommand = command;
        }
        return true;
    }

    if (event->key() == Qt::Key_Backspace) {
        m_input.chop(1);
        return true;
    }

    const QString text = event->text();
    if (!text.isEmpty()) {
        m_input += text;
    }
    return true;
}

void VimCommandSession::reset()
{
    m_active = false;
    m_input.clear();
    m_submittedCommand.reset();
}

std::optional<QString> VimCommandSession::takeSubmittedCommand()
{
    std::optional<QString> command = m_submittedCommand;
    m_submittedCommand.reset();
    return command;
}
