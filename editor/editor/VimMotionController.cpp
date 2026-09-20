#include "VimMotionController.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

VimMotionController::VimMotionController(QPlainTextEdit *editor)
    : QObject(editor), m_editor(editor) {}

void VimMotionController::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    m_count = 0;
    m_pendingFind = {};
    m_lastFindCharacter = {};
    m_waitingForG = false;
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

void VimMotionController::move(QTextCursor::MoveOperation operation, int count)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(operation, QTextCursor::MoveAnchor, count);
    m_editor->setTextCursor(cursor);
}

bool VimMotionController::findCharacter(QChar character, bool forward,
                                        bool till, int count)
{
    QTextCursor cursor = m_editor->textCursor();
    const QString text = cursor.block().text();
    const int current = cursor.positionInBlock();
    int index = current;
    for (int occurrence = 0; occurrence < count; ++occurrence) {
        index = forward ? text.indexOf(character, index + 1)
                        : text.lastIndexOf(character, index - 1);
        if (index < 0)
            return true;
    }
    if (till)
        index += forward ? -1 : 1;
    cursor.setPosition(cursor.block().position() + qMax(0, index));
    m_editor->setTextCursor(cursor);
    m_lastFindCharacter = character;
    m_lastFindForward = forward;
    m_lastFindTill = till;
    return true;
}

bool VimMotionController::repeatLastFind(bool reverseDirection, int count)
{
    if (m_lastFindCharacter.isNull())
        return true;
    return findCharacter(m_lastFindCharacter,
                         reverseDirection ? !m_lastFindForward
                                          : m_lastFindForward,
                         m_lastFindTill, count);
}

void VimMotionController::openLine(bool below)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(below ? QTextCursor::EndOfBlock
                              : QTextCursor::StartOfBlock);
    cursor.insertBlock(below ? QTextBlockFormat() : QTextBlockFormat());
    if (!below)
        cursor.movePosition(QTextCursor::Up);
    m_editor->setTextCursor(cursor);
    setMode(Mode::Insert);
}

bool VimMotionController::handleCommandMode(QKeyEvent *event)
{
    if (!m_commandMode)
        return false;

    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        const QString command = m_commandLine.trimmed();
        resetPending();
        if (!command.isEmpty())
            emit commandEntered(command);
        return true;
    }
    if (event->key() == Qt::Key_Backspace) {
        m_commandLine.chop(1);
        return true;
    }
    const QString text = event->text();
    if (!text.isEmpty())
        m_commandLine += text;
    return true;
}

bool VimMotionController::handleSearchMode(QKeyEvent *event)
{
    if (!m_searchMode)
        return false;

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Escape) {
        finishSearch();
        return true;
    }
    if (event->key() == Qt::Key_Backspace) {
        m_searchInput.chop(1);
        return true;
    }
    const QString text = event->text();
    if (!text.isEmpty())
        m_searchInput += text;
    return true;
}

bool VimMotionController::handlePendingOperation(QKeyEvent *event)
{
    if (m_pendingOp == PendingOp::None)
        return false;

    const QString text = event->text();
    const QChar key = text.isEmpty() ? QChar() : text.at(0);
    if (key == QLatin1Char('d') && m_pendingOp == PendingOp::Delete) {
        deleteLines(m_pendingOpCount);
        return true;
    }
    if (key == QLatin1Char('y') && m_pendingOp == PendingOp::Yank) {
        yankLines(m_pendingOpCount);
        return true;
    }
    if (key == QLatin1Char('c') && m_pendingOp == PendingOp::Change) {
        changeLines(m_pendingOpCount);
        return true;
    }
    if (key == QLatin1Char('g') && !m_waitingForG) {
        m_waitingForG = true;
        return true;
    }
    if (key == QLatin1Char('f') || key == QLatin1Char('F') ||
        key == QLatin1Char('t') || key == QLatin1Char('T')) {
        m_pendingFind = key;
        m_findForward = (key == QLatin1Char('f') ||
                         key == QLatin1Char('t'));
        m_findTill = (key == QLatin1Char('t') ||
                      key == QLatin1Char('T'));
        return true;
    }
    if (!m_pendingFind.isNull()) {
        const bool result = findCharacter(key, m_findForward, m_findTill,
                                          m_pendingOpCount);
        m_pendingFind = {};
        if (result) {
            QTextCursor anchor(m_editor->document());
            anchor.setPosition(m_pendingOpAnchor);
            QTextCursor target = m_editor->textCursor();
            target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
            applySelectionToPendingOp(target);
        } else {
            resetPending();
        }
        return true;
    }
    QTextCursor::MoveOperation operation;
    int count = 0;
    if (motionFromKey(key, operation, count)) {
        m_waitingForG = false;
        const int effectiveCount = count == 0 ? m_pendingOpCount : count;
        applyMotionToPendingOp(operation, effectiveCount);
        return true;
    }
    if (m_waitingForG) {
        m_waitingForG = false;
        if (key == QLatin1Char('g')) {
            QTextCursor anchor(m_editor->document());
            anchor.setPosition(m_pendingOpAnchor);
            QTextCursor target(m_editor->document());
            target.setPosition(0);
            target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
            applySelectionToPendingOp(target);
            return true;
        }
        if (key == QLatin1Char('G')) {
            QTextCursor anchor(m_editor->document());
            anchor.setPosition(m_pendingOpAnchor);
            QTextCursor target(m_editor->document());
            target.movePosition(QTextCursor::End);
            target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
            applySelectionToPendingOp(target);
            return true;
        }
    }
    resetPending();
    return true;
}

bool VimMotionController::motionFromKey(QChar key,
                                        QTextCursor::MoveOperation &operation,
                                        int &count)
{
    switch (key.unicode()) {
    case 'h':
        operation = QTextCursor::Left;
        count = 1;
        return true;
    case 'j':
        operation = QTextCursor::Down;
        count = 1;
        return true;
    case 'k':
        operation = QTextCursor::Up;
        count = 1;
        return true;
    case 'l':
        operation = QTextCursor::Right;
        count = 1;
        return true;
    case 'w':
    case 'W':
        operation = QTextCursor::NextWord;
        count = 1;
        return true;
    case 'b':
    case 'B':
        operation = QTextCursor::PreviousWord;
        count = 1;
        return true;
    case 'e':
        operation = QTextCursor::EndOfWord;
        count = 1;
        return true;
    case '0':
        operation = QTextCursor::StartOfBlock;
        count = 1;
        return true;
    case '$':
        operation = QTextCursor::EndOfBlock;
        count = 1;
        return true;
    default:
        return false;
    }
}

void VimMotionController::applyMotionToPendingOp(
    QTextCursor::MoveOperation operation, int count)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(operation, QTextCursor::MoveAnchor, count);
    m_editor->setTextCursor(cursor);

    QTextCursor anchor(m_editor->document());
    anchor.setPosition(m_pendingOpAnchor);
    QTextCursor target = m_editor->textCursor();
    target.setPosition(anchor.position(), QTextCursor::KeepAnchor);
    applySelectionToPendingOp(target);
}

void VimMotionController::applySelectionToPendingOp(
    const QTextCursor &selection)
{
    if (!selection.hasSelection())
        return;

    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(selection.selectionStart());
    cursor.setPosition(selection.selectionEnd(), QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
    if (m_pendingOp == PendingOp::Delete) {
        cursor.removeSelectedText();
    } else if (m_pendingOp == PendingOp::Yank) {
        QApplication::clipboard()->setText(selection.selectedText());
    } else if (m_pendingOp == PendingOp::Change) {
        cursor.removeSelectedText();
        setMode(Mode::Insert);
    }
    m_pendingOp = PendingOp::None;
    m_pendingOpCount = 1;
}

void VimMotionController::deleteLines(int count)
{
    QTextCursor cursor = m_editor->textCursor();
    const int block = cursor.blockNumber();
    const int endBlock = qMin(block + count - 1,
                              m_editor->document()->blockCount() - 1);
    QTextBlock startBlock = m_editor->document()->findBlockByNumber(block);
    QTextBlock endBlockText = m_editor->document()->findBlockByNumber(endBlock);
    const int start = startBlock.position();
    const int end =
        endBlock >= m_editor->document()->blockCount() - 1
            ? m_editor->document()->characterCount()
            : endBlockText.position() + endBlockText.length();
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    if (!cursor.hasSelection() && endBlock <
                                  m_editor->document()->blockCount() - 1)
        cursor.setPosition(end + 1, QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
    cursor.removeSelectedText();
    m_pendingOp = PendingOp::None;
    m_pendingOpCount = 1;
}

void VimMotionController::yankLines(int count)
{
    QTextCursor cursor = m_editor->textCursor();
    const int block = cursor.blockNumber();
    const int endBlock = qMin(block + count - 1,
                              m_editor->document()->blockCount() - 1);
    QTextBlock startBlock = m_editor->document()->findBlockByNumber(block);
    QTextBlock endBlockText = m_editor->document()->findBlockByNumber(endBlock);
    const int start = startBlock.position();
    const int end = endBlock >= m_editor->document()->blockCount() - 1
                        ? m_editor->document()->characterCount()
                        : endBlockText.position() + endBlockText.length() - 1;
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    QApplication::clipboard()->setText(cursor.selectedText());
    m_pendingOp = PendingOp::None;
    m_pendingOpCount = 1;
}

void VimMotionController::changeLines(int count)
{
    deleteLines(count);
    setMode(Mode::Insert);
}

void VimMotionController::deleteVisualSelection()
{
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return;
    cursor.removeSelectedText();
    m_visualMode = false;
}

void VimMotionController::yankVisualSelection()
{
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return;
    QApplication::clipboard()->setText(cursor.selectedText());
    m_visualMode = false;
}

void VimMotionController::changeVisualSelection()
{
    deleteVisualSelection();
    setMode(Mode::Insert);
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
        deleteVisualSelection();
        return true;
    }
    if (key == QLatin1Char('y')) {
        yankVisualSelection();
        return true;
    }
    if (key == QLatin1Char('c')) {
        changeVisualSelection();
        return true;
    }

    QTextCursor::MoveOperation operation;
    int count = 0;
    if (motionFromKey(key, operation, count)) {
        move(operation, count == 0 ? takeCount() : count);
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_visualAnchor, QTextCursor::KeepAnchor);
        m_editor->setTextCursor(cursor);
        return true;
    }
    return true;
}

void VimMotionController::startSearch(bool forward)
{
    m_searchMode = true;
    m_lastSearchForward = forward;
    m_searchInput.clear();
}

void VimMotionController::finishSearch()
{
    m_searchMode = false;
    if (m_searchInput.isEmpty()) {
        m_searchInput.clear();
        return;
    }
    m_lastSearch = m_searchInput;
    m_searchInput.clear();
    repeatSearch(m_lastSearchForward, 1);
}

void VimMotionController::repeatSearch(bool forward, int count)
{
    if (m_lastSearch.isEmpty())
        return;

    QTextCursor cursor = m_editor->textCursor();
    QTextDocument::FindFlags flags =
        forward ? QTextDocument::FindCaseSensitively
                : QTextDocument::FindBackward |
                      QTextDocument::FindCaseSensitively;
    for (int i = 0; i < count; ++i) {
        cursor = m_editor->document()->find(m_lastSearch, cursor, flags);
        if (cursor.isNull()) {
            cursor = m_editor->document()->find(
                m_lastSearch,
                forward ? 0 : m_editor->document()->characterCount(),
                flags);
        }
    }
    if (!cursor.isNull())
        m_editor->setTextCursor(cursor);
}

void VimMotionController::resetCount()
{
    m_count = 0;
    m_pendingFind = {};
    m_waitingForG = false;
}

void VimMotionController::resetPending()
{
    m_pendingOp = PendingOp::None;
    m_pendingOpCount = 1;
    m_pendingReplace = false;
    m_visualMode = false;
    m_commandMode = false;
    m_searchMode = false;
    m_commandLine.clear();
    m_searchInput.clear();
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
    if (handleSearchMode(event))
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
        startSearch(true);
        return true;
    }
    if (key == QLatin1Char('?')) {
        startSearch(false);
        return true;
    }
    if (key == QLatin1Char('n') || key == QLatin1Char('N')) {
        repeatSearch(key == QLatin1Char('n'), takeCount());
        return true;
    }
    if (key == QLatin1Char(':')) {
        m_commandMode = true;
        m_commandLine.clear();
        return true;
    }
    if (key == QLatin1Char('v')) {
        m_visualMode = true;
        m_visualAnchor = m_editor->textCursor().position();
        return true;
    }
    if (key == QLatin1Char('d') || key == QLatin1Char('y') ||
        key == QLatin1Char('c')) {
        m_pendingOp = key == QLatin1Char('d')
                          ? PendingOp::Delete
                          : (key == QLatin1Char('y') ? PendingOp::Yank
                                                     : PendingOp::Change);
        m_pendingOpAnchor = m_editor->textCursor().position();
        m_pendingOpCount = m_count == 0 ? 1 : m_count;
        m_count = 0;
        return true;
    }
    if (key == QLatin1Char('x')) {
        QTextCursor cursor = m_editor->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                            takeCount());
        cursor.removeSelectedText();
        cursor.endEditBlock();
        m_editor->setTextCursor(cursor);
        return true;
    }
    if (key == QLatin1Char('X')) {
        QTextCursor cursor = m_editor->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,
                            takeCount());
        cursor.removeSelectedText();
        cursor.endEditBlock();
        m_editor->setTextCursor(cursor);
        return true;
    }
    if (key == QLatin1Char('u')) {
        m_editor->undo();
        return true;
    }
    if (key == QLatin1Char('p') || key == QLatin1Char('P')) {
        QTextCursor cursor = m_editor->textCursor();
        if (key == QLatin1Char('p'))
            cursor.movePosition(QTextCursor::Right);
        m_editor->setTextCursor(cursor);
        m_editor->insertPlainText(QApplication::clipboard()->text());
        return true;
    }
    if (m_pendingReplace) {
        m_pendingReplace = false;
        QTextCursor cursor = m_editor->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                            takeCount());
        cursor.insertText(text);
        cursor.endEditBlock();
        m_editor->setTextCursor(cursor);
        return true;
    }
    if (key == QLatin1Char('r')) {
        m_pendingReplace = true;
        return true;
    }
    if (!m_pendingFind.isNull()) {
        const int count = takeCount();
        const bool result = findCharacter(key, m_findForward, m_findTill,
                                          count);
        m_pendingFind = {};
        return result;
    }
    if (m_waitingForG) {
        m_waitingForG = false;
        if (key == QLatin1Char('g')) {
            move(QTextCursor::Start, takeCount());
            return true;
        }
        return key == QLatin1Char('G');
    }
    if (key.isDigit() && (key != QLatin1Char('0') || m_count > 0)) {
        m_count = qMin(9999, m_count * 10 + key.digitValue());
        return true;
    }

    const int count = takeCount();
    switch (key.unicode()) {
    case 'h':
        move(QTextCursor::Left, count);
        return true;
    case 'j':
        move(QTextCursor::Down, count);
        return true;
    case 'k':
        move(QTextCursor::Up, count);
        return true;
    case 'l':
        move(QTextCursor::Right, count);
        return true;
    case 'w':
        move(QTextCursor::NextWord, count);
        return true;
    case 'W':
        move(QTextCursor::NextWord, count);
        return true;
    case 'b':
        move(QTextCursor::PreviousWord, count);
        return true;
    case 'B':
        move(QTextCursor::PreviousWord, count);
        return true;
    case 'e':
        move(QTextCursor::EndOfWord, count);
        return true;
    case '0':
        move(QTextCursor::StartOfBlock, 1);
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
        move(QTextCursor::EndOfBlock, 1);
        return true;
    case 'g':
        m_waitingForG = true;
        m_count = count == 1 ? 0 : count;
        return true;
    case 'G':
        move(QTextCursor::End, 1);
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
        return repeatLastFind(false, count);
    case ',':
        return repeatLastFind(true, count);
    case 'I':
        move(QTextCursor::StartOfBlock, 1);
        setMode(Mode::Insert);
        return true;
    case 'A':
        move(QTextCursor::EndOfBlock, 1);
        setMode(Mode::Insert);
        return true;
    case 'o':
        openLine(true);
        return true;
    case 'O':
        openLine(false);
        return true;
    case 'i':
        setMode(Mode::Insert);
        return true;
    case 'a':
        move(QTextCursor::Right, 1);
        setMode(Mode::Insert);
        return true;
    default:
        return true;
    }
}
