#include "VimCharacterSearch.h"

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>

VimCharacterSearch::VimCharacterSearch(QPlainTextEdit *editor)
    : m_editor(editor)
{
}

bool VimCharacterSearch::find(const VimCharacterSearchRequest &request)
{
    if (!m_editor || request.character.isNull() || request.count < 1)
        return false;

    QTextCursor cursor = m_editor->textCursor();
    const QString text = cursor.block().text();
    const qsizetype current = cursor.positionInBlock();
    qsizetype index = current;
    for (int occurrence = 0; occurrence < request.count; ++occurrence) {
        index = request.forward
                    ? text.indexOf(request.character, index + 1)
                    : text.lastIndexOf(request.character, index - 1);
        if (index < 0)
            return true;
    }

    if (request.till)
        index += request.forward ? -1 : 1;
    const qsizetype position =
        cursor.block().position() + qMax<qsizetype>(0, index);
    cursor.setPosition(static_cast<int>(position));
    m_editor->setTextCursor(cursor);
    m_lastCharacter = request.character;
    m_lastForward = request.forward;
    m_lastTill = request.till;
    return true;
}

bool VimCharacterSearch::repeatLast(bool reverseDirection, int count)
{
    if (m_lastCharacter.isNull())
        return true;

    return find({m_lastCharacter,
                 reverseDirection ? !m_lastForward : m_lastForward,
                 m_lastTill,
                 count});
}

void VimCharacterSearch::reset()
{
    m_lastCharacter = {};
    m_lastForward = true;
    m_lastTill = false;
}
