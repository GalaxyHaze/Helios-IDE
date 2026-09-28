#include "Code.h"
#include "EditorCompletionInsertionPolicy.h"
#include "EditorAppearanceController.h"
#include "EditorContextMenuController.h"
#include "EditorDecorationController.h"
#include "EditorDeletionPolicy.h"
#include "EditorTextEditApplier.h"
#include "EditorTypingPolicy.h"
#include "../core/SnippetManager.h"
#include "EditorLanguageFeatureController.h"
#include "LspCompletionModel.h"
#include "LspResultDecoder.h"
#include "VimMotionController.h"
#include <QAbstractItemView>
#include <QContextMenuEvent>
#include <QFileInfo>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolTip>
#include <QUrl>
#include <QWheelEvent>
#include <qnamespace.h>

static const int MIN_FONT_SIZE = 6;
static const int MAX_FONT_SIZE = 48;

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent),
      m_documentSyncController(
          {[this]() { return m_lspClient && m_lspClient->isReady(); },
           [this]() {
             return m_lspClient ? m_lspClient->documentSyncKind() : 1;
           },
           [this](const QString &uri, const QList<LspTextChange> &changes,
                  int version) {
             return m_lspClient &&
                    m_lspClient->changeDocument(uri, changes, version);
           },
           [this](const QString &uri, const QString &text, int version) {
           return m_lspClient &&
                  m_lspClient->changeDocumentFull(uri, text, version);
           }}) {
  setFrameShape(QFrame::NoFrame);
  lineNumberArea = new LineNumberArea(this);
  m_appearanceController = new EditorAppearanceController(this, this);
  m_decorationController =
      new EditorDecorationController(this, m_appearanceController, this);
  connect(m_appearanceController,
          &EditorAppearanceController::appearanceChanged, this, [this]() {
            if (m_decorationController)
              m_decorationController->refreshAppearance();
            updateLineNumberAreaWidth(0);
            lineNumberArea->update();
            viewport()->update();
          });
  m_vimController = new VimMotionController(this);
  connect(m_vimController, &VimMotionController::modeChanged, this,
          [this](VimMotionController::Mode mode) {
            QString label = "OFF";
            if (mode == VimMotionController::Mode::Normal)
              label = "NORMAL";
            else if (mode == VimMotionController::Mode::Insert)
              label = "INSERT";
            emit vimModeChanged(label);
          });
  connect(m_vimController, &VimMotionController::commandEntered, this,
          [this](const QString &command) { emit vimCommandEntered(command); });

  updateTheme();

  setCursorWidth(2);
  viewport()->setMouseTracking(true);

  m_languageFeatures = new EditorLanguageFeatureController(this, this);
  m_contextMenuController =
      new EditorContextMenuController(this, m_languageFeatures, this);

  connect(this, &CodeEditor::blockCountChanged, this,
          &CodeEditor::updateLineNumberAreaWidth);
  connect(this, &CodeEditor::updateRequest, this,
          &CodeEditor::updateLineNumberArea);
  connect(this, &CodeEditor::cursorPositionChanged, this, [this]() {
    if (m_decorationController) {
      m_decorationController->refreshCursorDecorations();
      m_decorationController->clearLspHighlights();
    }
    if (m_languageFeatures)
      m_languageFeatures->handleCursorPositionChanged();
  });
  connect(document(), &QTextDocument::contentsChange, this,
          &CodeEditor::onDocumentContentsChanged);

  updateLineNumberAreaWidth(0);
  if (m_decorationController)
    m_decorationController->refreshAppearance();
}

void CodeEditor::setVimMotionsEnabled(bool enabled)
{
  m_vimController->setEnabled(enabled);
  emit vimModeChanged(enabled ? "NORMAL" : "OFF");
}

bool CodeEditor::vimMotionsEnabled() const
{
  return m_vimController && m_vimController->isEnabled();
}

CodeEditor::~CodeEditor() {
  m_suppressDocumentSync = true;
  m_documentSyncController.stop();
  if (document()) {
    document()->disconnect(this);
  }
}

const EditorAppearance &CodeEditor::editorAppearance() const
{
  return m_appearanceController->appearance();
}

void CodeEditor::updateTheme() {
  if (m_appearanceController)
    m_appearanceController->apply();
}

void CodeEditor::setFilePath(const QString &path) {
  m_filePath = path;
  m_fileUri = QUrl::fromLocalFile(path).toString();
  m_documentSyncController.setDocument(m_fileUri, toPlainText(),
                                       m_documentSyncController.version());
}

void CodeEditor::setInitialDocumentText(const QString &text,
                                        int initialVersion) {
  m_suppressDocumentSync = true;
  setPlainText(text);
  m_suppressDocumentSync = false;
  m_documentSyncController.setDocument(m_fileUri, text, initialVersion);
}

void CodeEditor::setLspClient(LspClient *client) {
  if (m_lspClient == client)
    return;

  detachLspClient();
  m_lspClient = client;
  if (m_languageFeatures)
    m_languageFeatures->setClient(client);
}

EditorLanguageRequestContext CodeEditor::currentLanguageRequest() {
  flushDocumentChanges();
  if (m_fileUri.isEmpty())
    return {};

  const QTextCursor cursor = textCursor();
  return {m_fileUri,
          documentVersion(),
          {cursor.blockNumber(), cursor.positionInBlock()}};
}

bool CodeEditor::prepareCompletion() {
  if (!m_completer)
    return false;
  m_completer->setWidget(this);
  return true;
}

bool CodeEditor::handleCompletionKey(QKeyEvent *event) {
  if (!event || !m_completer ||
      !m_completer->popup()->isVisible()) {
    return false;
  }

  switch (event->key()) {
  case Qt::Key_Enter:
  case Qt::Key_Return:
  case Qt::Key_Tab:
    if (!m_completer->currentCompletion().isEmpty()) {
      onCompletionSelected(m_completer->insertText(),
                           m_completer->insertTextFormat());
      return true;
    }
    break;
  case Qt::Key_Escape:
    m_completer->popup()->hide();
    return true;
  case Qt::Key_Up:
  case Qt::Key_Down:
  case Qt::Key_PageUp:
  case Qt::Key_PageDown:
    break;
  default:
    break;
  }
  return false;
}

void CodeEditor::detachLspClient() {
  if (m_languageFeatures)
    m_languageFeatures->detachClient();
  m_lspClient = nullptr;
  m_documentSyncController.discardPendingChanges();
  if (m_decorationController)
    m_decorationController->clearLspHighlights();
  if (m_decorationController)
    m_decorationController->clearSemanticTokens();
  clearFoldingRanges();
  clearDiagnostics();
}

void CodeEditor::markLspDocumentSynchronized()
{
  m_documentSyncController.markDocumentSynchronized();
}

void CodeEditor::setCompleter(LspCompleter *completer) {
  m_completer = completer;

  if (m_completer) {
    m_completer->setWidget(this);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, [this](const QString &text) {
              if (m_completer->widget() == this) {
                const LspCompletionItem item = m_completer->currentItem();
                if (!item.label.isEmpty())
                    applyCompletionItem(item);
                else
                    onCompletionSelected(text,
                                         m_completer->insertTextFormat());
              }
            });
  }
}

// ── Diagnostics ──────────────────────────────────────────────────────

void CodeEditor::setDiagnostics(const QList<LspDiagnostic> &diagnostics) {
  m_diagnostics = diagnostics;
  if (m_decorationController)
    m_decorationController->setDiagnostics(diagnostics);
}

void CodeEditor::clearDiagnostics() {
  m_diagnostics.clear();
  if (m_decorationController)
    m_decorationController->setDiagnostics({});
}

void CodeEditor::setSemanticTokens(
    const QList<LspSemanticToken> &tokens) {
  if (m_decorationController)
    m_decorationController->setSemanticTokens(tokens);
}

void CodeEditor::clearSemanticTokens() {
  if (m_decorationController)
    m_decorationController->clearSemanticTokens();
}

void CodeEditor::setFoldingRanges(
    const QList<LspFoldingRange> &ranges) {
  clearFoldingRanges();
  m_foldingRanges = ranges;
  lineNumberArea->update();
}

void CodeEditor::clearFoldingRanges() {
  for (auto it = m_hiddenLineCounts.cbegin();
       it != m_hiddenLineCounts.cend(); ++it) {
    QTextBlock block = document()->findBlockByNumber(it.key());
    if (!block.isValid())
      continue;
    block.setVisible(true);
    block.setLineCount(it.value());
  }
  m_hiddenLineCounts.clear();
  document()->markContentsDirty(0, document()->characterCount());
  viewport()->update();
  lineNumberArea->update();
}

bool CodeEditor::hasFoldingRangeAtLine(int line) const
{
  for (const LspFoldingRange &range : m_foldingRanges) {
    if (range.startLine == line)
      return true;
  }
  return false;
}

bool CodeEditor::isFoldedAtLine(int line) const
{
  for (const LspFoldingRange &range : m_foldingRanges) {
    if (range.startLine != line)
      continue;
    const QTextBlock firstHidden =
        document()->findBlockByNumber(range.startLine + 1);
    return firstHidden.isValid() && !firstHidden.isVisible();
  }
  return false;
}

void CodeEditor::toggleFoldAtLine(int line)
{
  const LspFoldingRange *selected = nullptr;
  for (const LspFoldingRange &range : m_foldingRanges) {
    if (range.startLine != line)
      continue;
    if (!selected || range.endLine > selected->endLine)
      selected = &range;
  }
  if (!selected)
    return;

  const QTextBlock firstHidden =
      document()->findBlockByNumber(selected->startLine + 1);
  const bool unfold = firstHidden.isValid() && !firstHidden.isVisible();
  const int lastLine =
      qMin(selected->endLine, document()->blockCount() - 1);
  for (int currentLine = selected->startLine + 1;
       currentLine <= lastLine; ++currentLine) {
    QTextBlock block = document()->findBlockByNumber(currentLine);
    if (!block.isValid())
      continue;
    if (unfold) {
      block.setVisible(true);
      block.setLineCount(m_hiddenLineCounts.value(
          currentLine, qMax(1, block.lineCount())));
      m_hiddenLineCounts.remove(currentLine);
    } else if (block.isVisible()) {
      m_hiddenLineCounts.insert(currentLine,
                                qMax(1, block.lineCount()));
      block.setVisible(false);
      block.setLineCount(0);
    }
  }
  document()->markContentsDirty(0, document()->characterCount());
  viewport()->update();
  lineNumberArea->update();
}

void CodeEditor::setLspHighlightRanges(const QList<LspRange> &ranges) {
  if (m_decorationController)
    m_decorationController->setLspHighlightRanges(ranges);
}

// ── Line Number Area ─────────────────────────────────────────────────

int CodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  return 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), editorAppearance().gutterBackground);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  int cursorLine = textCursor().blockNumber();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      bool isCurrent = (blockNumber == cursorLine);
      painter.setPen(isCurrent ? editorAppearance().gutterActive
                               : editorAppearance().lineNumber);

      QFont f = font();
      f.setBold(isCurrent);
      painter.setFont(f);

      const bool foldable = hasFoldingRangeAtLine(blockNumber);
      if (foldable) {
        painter.setPen(editorAppearance().lineNumber);
        painter.drawText(0, top, 8, fontMetrics().height(), Qt::AlignCenter,
                         isFoldedAtLine(blockNumber) ? QStringLiteral(">")
                                                     : QStringLiteral("v"));
        painter.setPen(isCurrent ? editorAppearance().gutterActive
                                 : editorAppearance().lineNumber);
      }
      painter.drawText(foldable ? 8 : 0, top, lineNumberArea->width() - 8,
                       fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }

  // Gutter separator line
  painter.setPen(editorAppearance().border);
  int w = lineNumberArea->width();
  painter.drawLine(w - 1, event->rect().top(), w - 1, event->rect().bottom());
}

void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
  if (event && event->button() == Qt::LeftButton &&
      event->position().x() < 12) {
    const QTextCursor cursor =
        codeEditor->cursorForPosition(
            QPoint(1, qRound(event->position().y())));
    if (codeEditor->hasFoldingRangeAtLine(cursor.blockNumber())) {
      codeEditor->toggleFoldAtLine(cursor.blockNumber());
      event->accept();
      return;
    }
  }
  QWidget::mousePressEvent(event);
}

// ── Key Handling ─────────────────────────────────────────────────────

void CodeEditor::keyPressEvent(QKeyEvent *e) {
  if (m_languageFeatures && m_languageFeatures->handleKeyPress(e))
    return;

  // Global actions and completion handling deliberately precede Vim.  In
  // Insert mode the controller returns false, preserving all editor behavior.
  if (m_vimController && m_vimController->handleKeyPress(e))
    return;

  // ── Smart backspace ────────────────────────────────────────
  if (e->key() == Qt::Key_Backspace) {
    QTextCursor cursor = textCursor();
    const int position = cursor.position();
    const QChar nextCharacter =
        position < document()->characterCount()
            ? document()->characterAt(position)
            : QChar();
    const DeletionDecision deletion =
        EditorDeletionPolicy::decide(
            cursor.block().text().left(cursor.positionInBlock()),
            nextCharacter, cursor.hasSelection());

    if (deletion.action == DeletionDecision::Action::DeletePair) {
      cursor.setPosition(position - 1);
      cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
      cursor.removeSelectedText();
      return;
    }

    if (deletion.action == DeletionDecision::Action::DeleteIndentation) {
      for (int i = 0; i < deletion.characterCount; ++i)
        cursor.deletePreviousChar();
      return;
    }

    QPlainTextEdit::keyPressEvent(e);
    return;
  }

  // ── Snippet expansion on Tab ───────────────────────────────
  if (e->key() == Qt::Key_Tab && !e->modifiers()) {
    if (tryExpandSnippet())
      return;
  }

  // ── Tab → 4 spaces ─────────────────────────────────────────
  if (e->key() == Qt::Key_Tab) {
    insertPlainText("    ");
    return;
  }

  // ── Auto-indent on Enter ───────────────────────────────────
  if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
    autoIndent();
    return;
  }

  // ── Zoom ────────────────────────────────────────────────────
  if (e->modifiers() == Qt::ControlModifier) {
    if (e->key() == Qt::Key_Equal || e->key() == Qt::Key_Plus) {
      zoomIn();
      return;
    }
    if (e->key() == Qt::Key_Minus) {
      zoomOut();
      return;
    }
    if (e->key() == Qt::Key_0) {
      zoomReset();
      return;
    }
  }

  // ── Auto-dedent on } ───────────────────────────────────────
  if (e->text() == "}") {
    QTextCursor cursor = textCursor();
    QString text = cursor.block().text();
    int pos = cursor.positionInBlock();
    QString before = text.left(pos);
    QString after = text.mid(pos);
    if (!before.isEmpty() && before.trimmed().isEmpty() &&
        after.trimmed().isEmpty() && before.length() >= 4) {
      for (int i = 0; i < 4; ++i)
        cursor.deletePreviousChar();
      setTextCursor(cursor);
    }
    QPlainTextEdit::keyPressEvent(e);
    return;
  }

  // ── Auto-close pairs ────────────────────────────────────────
  if (!e->text().isEmpty() &&
      !(e->modifiers() &
        (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
    QChar ch = e->text().at(0);
    if (EditorTypingPolicy::isAutoCloseCharacter(ch) &&
        handleAutoClose(ch))
      return;
  }

  // ── Default handling ────────────────────────────────────────
  QPlainTextEdit::keyPressEvent(e);

  // ── Post-event: trigger completions / signature help ────────
  if (m_completer) {
    QString text = textCursor().block().text();
    int pos = textCursor().positionInBlock();
    if (pos > 0) {
      QChar ch = text.at(pos - 1);
      if (ch == '.' || ch == ':')
        triggerCompletion();
    }
  }

  if (e->key() == Qt::Key_ParenLeft && m_lspClient)
    triggerSignatureHelp();
}

void CodeEditor::autoIndent() {
  QTextCursor cursor = textCursor();
  insertPlainText("\n" +
                  EditorTypingPolicy::indentationForLine(cursor.block().text()));
}

bool CodeEditor::handleAutoClose(QChar ch) {
  QTextCursor cursor = textCursor();
  const int position = cursor.position();
  const QChar previous =
      position > 0 ? document()->characterAt(position - 1) : QChar();
  const QChar next = document()->characterAt(position);
  const AutoCloseDecision decision =
      EditorTypingPolicy::autoCloseDecision(ch, previous, next,
                                            cursor.hasSelection());
  if (!decision.handled())
    return false;

  if (decision.action == AutoCloseDecision::Action::SurroundSelection) {
    const QChar close = decision.closing;
    QString selected = cursor.selectedText();
    cursor.insertText(QString(ch) + selected + close);
    cursor.setPosition(cursor.position() - 1);
    setTextCursor(cursor);
    if (ch == '(' && m_lspClient)
      triggerSignatureHelp();
    return true;
  }

  if (decision.action == AutoCloseDecision::Action::JumpOver) {
    cursor.movePosition(QTextCursor::Right);
    setTextCursor(cursor);
    return true;
  }

  insertPlainText(QString(ch) + decision.closing);
  cursor = textCursor();
  cursor.movePosition(QTextCursor::Left);
  setTextCursor(cursor);

  if (ch == '(' && m_lspClient)
    triggerSignatureHelp();

  return true;
}

void CodeEditor::mousePressEvent(QMouseEvent *e) {
  if (m_languageFeatures && m_languageFeatures->handleMousePress(e))
    return;

  QPlainTextEdit::mousePressEvent(e);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *e) {
  if (e->modifiers() == Qt::ControlModifier) {
    viewport()->setCursor(Qt::PointingHandCursor);
  } else {
    viewport()->setCursor(Qt::IBeamCursor);
  }

  if (m_languageFeatures)
    m_languageFeatures->handleMouseMove(e);

  QPlainTextEdit::mouseMoveEvent(e);
}

bool CodeEditor::event(QEvent *e) { return QPlainTextEdit::event(e); }

// ── Completion ───────────────────────────────────────────────────────

void CodeEditor::triggerCompletion() {
  if (m_languageFeatures)
    m_languageFeatures->triggerCompletion();
}

void CodeEditor::onCompletionSelected(const QString &insertText,
                                      int insertTextFormat) {
  const QTextCursor currentCursor = textCursor();
  const auto decision = EditorCompletionInsertionPolicy::prepare(
      currentCursor.block().text(), currentCursor.positionInBlock(), insertText,
      insertTextFormat);
  if (!decision.valid)
    return;

  QTextCursor cursor = currentCursor;
  const int blockStart = currentCursor.block().position();
  cursor.setPosition(blockStart + decision.start);
  cursor.setPosition(blockStart + decision.end, QTextCursor::KeepAnchor);
  cursor.insertText(decision.text);
  setTextCursor(cursor);
}

void CodeEditor::applyCompletionItem(const LspCompletionItem &item)
{
  const QJsonObject raw = item.rawItem;
  const QJsonObject textEdit = raw.value(QStringLiteral("textEdit"))
                                   .toObject();
  if (textEdit.isEmpty()) {
    onCompletionSelected(item.insertText, item.insertTextFormat);
    return;
  }

  QJsonObject rangeValue = textEdit.value(QStringLiteral("range")).toObject();
  if (rangeValue.isEmpty())
    rangeValue = textEdit.value(QStringLiteral("replace")).toObject();
  if (rangeValue.isEmpty()) {
    onCompletionSelected(item.insertText, item.insertTextFormat);
    return;
  }

  const LspRange range = LspResultDecoder::range(rangeValue);
  const QString newText =
      textEdit.value(QStringLiteral("newText")).toString(item.insertText);
  const auto mainDecision = EditorCompletionInsertionPolicy::prepareRange(
      offsetForLspPosition(range.start), offsetForLspPosition(range.end),
      newText, item.insertTextFormat);
  if (!mainDecision.valid)
    return;

  QList<EditorTextEditApplier::TextEdit> edits =
      LspResultDecoder::textEdits(
          raw.value(QStringLiteral("additionalTextEdits")));
  const int mainStart =
      EditorTextEditApplier::offsetForLspPosition(*document(), range.start);
  const int mainEnd =
      EditorTextEditApplier::offsetForLspPosition(*document(), range.end);
  int cursorOffset = mainStart + mainDecision.text.size();
  for (const auto &edit : edits) {
    const int editStart =
        EditorTextEditApplier::offsetForLspPosition(*document(),
                                                    edit.first.start);
    const int editEnd =
        EditorTextEditApplier::offsetForLspPosition(*document(),
                                                    edit.first.end);
    if (editStart < mainEnd && mainStart < editEnd)
      return;
    if (editEnd <= mainStart)
      cursorOffset += edit.second.size() - (editEnd - editStart);
  }
  edits.append({range, mainDecision.text});
  EditorTextEditApplier::apply(*document(), edits);

  QTextCursor cursor(document());
  cursor.setPosition(qBound(0, cursorOffset, document()->characterCount() - 1));
  setTextCursor(cursor);
}

void CodeEditor::triggerSignatureHelp() {
  if (m_languageFeatures)
    m_languageFeatures->triggerSignatureHelp();
}

void CodeEditor::setFindSelections(
    const QList<QTextEdit::ExtraSelection> &selections) {
  if (m_decorationController)
    m_decorationController->setFindSelections(selections);
}

void CodeEditor::updateDiagnosticDisplay() {
  if (m_decorationController)
    m_decorationController->refreshAppearance();
}

// ── Document Sync ───────────────────────────────────────────────────

void CodeEditor::onDocumentContentsChanged(int position, int charsRemoved,
                                           int charsAdded) {
  if (m_suppressDocumentSync || m_fileUri.isEmpty())
    return;

  QTextCursor cursor(document());
  cursor.setPosition(position);
  cursor.setPosition(position + charsAdded, QTextCursor::KeepAnchor);
  QString insertedText = cursor.selectedText();
  insertedText.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));

  m_documentSyncController.recordChange(
      {position, charsRemoved, insertedText, toPlainText()});
  clearDiagnostics();
  if (m_decorationController)
    m_decorationController->clearLspHighlights();
  if (m_decorationController)
    m_decorationController->clearSemanticTokens();
  clearFoldingRanges();
}

void CodeEditor::flushDocumentChanges() {
  m_documentSyncController.flush();
}

void CodeEditor::goToLine(int line, int character) {
  QTextBlock block = document()->findBlockByNumber(line);
  if (!block.isValid())
    return;

  QTextCursor cursor(block);
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, character);
  setTextCursor(cursor);
  centerCursor();
  setFocus();
}

// ── Snippet Expansion ──────────────────────────────────────────────

void CodeEditor::setSnippetManager(SnippetManager *manager) {
  m_snippetManager = manager;
}

bool CodeEditor::tryExpandSnippet() {
  if (!m_snippetManager)
    return false;

  QTextCursor cursor = textCursor();
  int pos = cursor.positionInBlock();
  QString text = cursor.block().text().left(pos);

  int wordStart = pos;
  while (wordStart > 0 &&
         (text[wordStart - 1].isLetterOrNumber() ||
          text[wordStart - 1] == '-' || text[wordStart - 1] == '_'))
    wordStart--;

  QString prefix = text.mid(wordStart, pos - wordStart);
  if (prefix.isEmpty())
    return false;

  QString body = m_snippetManager->expand(prefix);
  if (body.isEmpty())
    return false;

  cursor.setPosition(cursor.block().position() + wordStart);
  cursor.setPosition(cursor.block().position() + pos, QTextCursor::KeepAnchor);
  cursor.insertText(body);
  setTextCursor(cursor);
  return true;
}

// ── Zoom ────────────────────────────────────────────────────────────

static const int DEFAULT_FONT_SIZE = 12;

void CodeEditor::zoomIn() {
  QFont f = font();
  int sz = f.pointSize();
  if (sz < MAX_FONT_SIZE)
    f.setPointSize(sz + 1);
  setFont(f);
  updateLineNumberAreaWidth(0);
  emit zoomChanged(double(f.pointSize()) / DEFAULT_FONT_SIZE);
}

void CodeEditor::zoomOut() {
  QFont f = font();
  int sz = f.pointSize();
  if (sz > MIN_FONT_SIZE)
    f.setPointSize(sz - 1);
  setFont(f);
  updateLineNumberAreaWidth(0);
  emit zoomChanged(double(f.pointSize()) / DEFAULT_FONT_SIZE);
}

void CodeEditor::zoomReset() {
  QFont f = font();
  f.setPointSize(DEFAULT_FONT_SIZE);
  setFont(f);
  updateLineNumberAreaWidth(0);
  emit zoomChanged(1.0);
}

void CodeEditor::wheelEvent(QWheelEvent *e) {
  if (e->modifiers() == Qt::ControlModifier) {
    int delta = e->angleDelta().y();
    if (delta > 0)
      zoomIn();
    else if (delta < 0)
      zoomOut();
    e->accept();
    return;
  }
  QPlainTextEdit::wheelEvent(e);
}

int CodeEditor::offsetForLspPosition(const LspPosition &pos) const {
  return EditorTextEditApplier::offsetForLspPosition(*document(), pos);
}

void CodeEditor::applyEdits(const QList<QPair<LspRange, QString>> &edits) {
  EditorTextEditApplier::apply(*document(), edits);
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event) {
  if (m_contextMenuController)
    m_contextMenuController->show(event->globalPos());
}
