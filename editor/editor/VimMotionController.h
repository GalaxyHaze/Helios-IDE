#ifndef VIMMOTIONCONTROLLER_H
#define VIMMOTIONCONTROLLER_H

#include <QObject>
#include <QTextCursor>

class QKeyEvent;
class QPlainTextEdit;

class VimMotionController : public QObject {
  Q_OBJECT

public:
  enum class Mode { Off, Normal, Insert };

  explicit VimMotionController(QPlainTextEdit *editor);
  void setEnabled(bool enabled);
  bool isEnabled() const { return m_enabled; }
  Mode mode() const { return m_enabled ? m_mode : Mode::Off; }
  bool handleKeyPress(QKeyEvent *event);

signals:
  void modeChanged(VimMotionController::Mode mode);

private:
  void setMode(Mode mode);
  int takeCount();
  void move(QTextCursor::MoveOperation operation, int count);
  bool findCharacter(QChar character, bool forward, bool till, int count);
  bool repeatLastFind(bool reverseDirection, int count);
  void openLine(bool below);

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
};

#endif
