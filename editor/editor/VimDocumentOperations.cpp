#include "VimDocumentOperations.h"

#include <QApplication>
#include <QClipboard>
#include <QPlainTextEdit>
#include <QTextBlock>

VimDocumentOperations::VimDocumentOperations(QPlainTextEdit *editor)
    : m_editor(editor)
{
}

void VimDocumentOperations::move(QTextCursor::MoveOperation operation,
                                 int count)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(operation, QTextCursor::MoveAnchor, count);
    m_editor->setTextCursor(cursor);
}

void VimDocumentOperations::openLine(bool below)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(below ? QTextCursor::EndOfBlock
                              : QTextCursor::StartOfBlock);
    cursor.insertBlock(QTextBlockFormat());
    if (!below)
        cursor.movePosition(QTextCursor::Up);
    m_editor->setTextCursor(cursor);
}

bool VimDocumentOperations::applySelection(
    const QTextCursor &selection, VimPendingOperation operation)
{
    if (!selection.hasSelection())
        return false;

    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(selection.selectionStart());
    cursor.setPosition(selection.selectionEnd(), QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
    if (operation == VimPendingOperation::Yank) {
        QApplication::clipboard()->setText(selection.selectedText());
    } else {
        cursor.removeSelectedText();
    }
    return operation == VimPendingOperation::Change;
}

void VimDocumentOperations::deleteLines(int count)
{
    QTextCursor cursor = m_editor->textCursor();
    const int block = cursor.blockNumber();
    const int endBlock = qMin(
        block + count - 1, m_editor->document()->blockCount() - 1);
    const QTextBlock startBlock =
        m_editor->document()->findBlockByNumber(block);
    const QTextBlock endBlockText =
        m_editor->document()->findBlockByNumber(endBlock);
    const int start = startBlock.position();
    const int end =
        endBlock >= m_editor->document()->blockCount() - 1
            ? m_editor->document()->characterCount()
            : endBlockText.position() + endBlockText.length();
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    if (!cursor.hasSelection() &&
        endBlock < m_editor->document()->blockCount() - 1) {
        cursor.setPosition(end + 1, QTextCursor::KeepAnchor);
    }
    m_editor->setTextCursor(cursor);
    cursor.removeSelectedText();
}

void VimDocumentOperations::yankLines(int count)
{
    QTextCursor cursor = m_editor->textCursor();
    const int block = cursor.blockNumber();
    const int endBlock = qMin(
        block + count - 1, m_editor->document()->blockCount() - 1);
    const QTextBlock startBlock =
        m_editor->document()->findBlockByNumber(block);
    const QTextBlock endBlockText =
        m_editor->document()->findBlockByNumber(endBlock);
    const int start = startBlock.position();
    const int end = endBlock >= m_editor->document()->blockCount() - 1
                        ? m_editor->document()->characterCount()
                        : endBlockText.position() + endBlockText.length() - 1;
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    QApplication::clipboard()->setText(cursor.selectedText());
}

void VimDocumentOperations::changeLines(int count)
{
    deleteLines(count);
}

void VimDocumentOperations::deleteSelection()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
}

void VimDocumentOperations::yankSelection()
{
    const QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection())
        QApplication::clipboard()->setText(cursor.selectedText());
}

void VimDocumentOperations::deleteCharacters(int count, bool backward)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(backward ? QTextCursor::Left : QTextCursor::Right,
                        QTextCursor::KeepAnchor, count);
    cursor.removeSelectedText();
    cursor.endEditBlock();
    m_editor->setTextCursor(cursor);
}

void VimDocumentOperations::paste(bool after)
{
    QTextCursor cursor = m_editor->textCursor();
    if (after)
        cursor.movePosition(QTextCursor::Right);
    m_editor->setTextCursor(cursor);
    m_editor->insertPlainText(QApplication::clipboard()->text());
}

void VimDocumentOperations::replaceCharacters(const QString &text, int count)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, count);
    cursor.insertText(text);
    cursor.endEditBlock();
    m_editor->setTextCursor(cursor);
}
