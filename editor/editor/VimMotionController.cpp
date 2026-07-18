#include "VimMotionController.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>

VimMotionController::VimMotionController(QPlainTextEdit *editor)
    : QObject(editor), m_editor(editor) {}

void VimMotionController::setEnabled(bool enabled) {
  if (m_enabled == enabled)
    return;
  m_enabled = enabled;
  m_count = 0;
  m_pendingFind = {};
  m_lastFindCharacter = {};
  m_waitingForG = false;
  setMode(enabled ? Mode::Normal : Mode::Off);
}

void VimMotionController::setMode(Mode mode) {
  if (m_mode == mode && (mode != Mode::Off || !m_enabled))
    return;
  m_mode = mode;
  emit modeChanged(this->mode());
}

int VimMotionController::takeCount() {
  const int count = m_count == 0 ? 1 : m_count;
  m_count = 0;
  return count;
}

void VimMotionController::move(QTextCursor::MoveOperation operation,
                               int count) {
  QTextCursor cursor = m_editor->textCursor();
  cursor.movePosition(operation, QTextCursor::MoveAnchor, count);
  m_editor->setTextCursor(cursor);
}

bool VimMotionController::findCharacter(QChar character, bool forward,
                                        bool till, int count) {
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

bool VimMotionController::repeatLastFind(bool reverseDirection, int count) {
  if (m_lastFindCharacter.isNull())
    return true;
  return findCharacter(m_lastFindCharacter,
                       reverseDirection ? !m_lastFindForward
                                        : m_lastFindForward,
                       m_lastFindTill, count);
}

void VimMotionController::openLine(bool below) {
  QTextCursor cursor = m_editor->textCursor();
  cursor.movePosition(below ? QTextCursor::EndOfBlock
                            : QTextCursor::StartOfBlock);
  cursor.insertBlock(below ? QTextBlockFormat() : QTextBlockFormat());
  if (!below)
    cursor.movePosition(QTextCursor::Up);
  m_editor->setTextCursor(cursor);
  setMode(Mode::Insert);
}

bool VimMotionController::handleKeyPress(QKeyEvent *event) {
  if (!m_enabled || !m_editor)
    return false;

  if (m_mode == Mode::Insert) {
    if (event->key() == Qt::Key_Escape) {
      setMode(Mode::Normal);
      return true;
    }
    return false;
  }

  if (event->key() == Qt::Key_Escape) {
    m_count = 0;
    m_pendingFind = {};
    m_waitingForG = false;
    return true;
  }
  if (event->modifiers() != Qt::NoModifier)
    return false;

  const QString text = event->text();
  if (text.isEmpty())
    return false;
  const QChar key = text.at(0);
  if (!m_pendingFind.isNull()) {
    const int count = takeCount();
    const bool result = findCharacter(key, m_findForward, m_findTill, count);
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
