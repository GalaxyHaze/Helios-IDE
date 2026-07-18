#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "LspClient.h"
#include <QList>
#include <QMap>
#include <QPlainTextEdit>
#include <QTimer>
#include <QWidget>

class LspCompleter;
class SnippetManager;
class LineNumberArea;
class VimMotionController;

class CodeEditor : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit CodeEditor(QWidget *parent = nullptr);
  ~CodeEditor() override;

  void setFilePath(const QString &path);
  void setInitialDocumentText(const QString &text, int initialVersion = 1);
  QString filePath() const { return m_filePath; }
  QString fileUri() const { return m_fileUri; }
  int documentVersion() const { return m_documentVersion; }
  QList<LspDiagnostic> diagnostics() const { return m_diagnostics; }
  void flushPendingLspChanges() { flushDocumentChanges(); }

  void setLspClient(LspClient *client);
  void setCompleter(LspCompleter *completer);
  void setSnippetManager(SnippetManager *manager);

  void setDiagnostics(const QList<LspDiagnostic> &diagnostics);
  void clearDiagnostics();

  void updateDiagnosticDisplay();

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  void goToLine(int line, int character = 0);
  int offsetForLspPosition(const LspPosition &pos) const;
  void applyEdits(const QList<QPair<LspRange, QString>> &edits);
  void setFindSelections(const QList<QTextEdit::ExtraSelection> &selections);
  void setVimMotionsEnabled(bool enabled);
  bool vimMotionsEnabled() const;

  friend class LineNumberArea;

signals:
  void navigateToLocation(const QString &uri, int line, int character);
  void renameRequested(const QString &uri, int version,
                       const LspPosition &position, const QString &newName);
  void codeActionsRequested(const QString &uri, int version,
                            const LspRange &range);
  void zoomChanged(double scaleFactor);
  void vimModeChanged(const QString &mode);

public slots:
  void onCompletionSelected(const QString &insertText,
                            int insertTextFormat = 1);
  void zoomIn();
  void zoomOut();
  void zoomReset();

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  bool event(QEvent *e) override;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(const QRect &, int);
  void highlightCurrentLine();
  void onDocumentContentsChanged(int position, int charsRemoved,
                                 int charsAdded);
  void flushDocumentChanges();
  void updateTheme();

private:
  void triggerCompletion();
  void triggerSignatureHelp();
  void goToDefinitionAtCursor();
  void goToImplementationAtCursor();
  void goToDeclarationAtCursor();
  void requestHoverAtCursor();
  void replaceCurrentWord(const QString &insertText);
  void updateDiagnosticHighlights();
  void matchBrackets();
  void onHoverTimeout();
  void autoIndent();
  bool handleAutoClose(QChar ch);
  bool isAutoCloseChar(QChar ch) const;
  bool tryExpandSnippet();
  LspPosition lspPositionForOffset(const QString &text, int offset) const;
  LspPosition lspPositionForOffset(int offset) const;

  LineNumberArea *lineNumberArea;
  LspClient *m_lspClient = nullptr;
  LspCompleter *m_completer = nullptr;
  SnippetManager *m_snippetManager = nullptr;

  QString m_filePath;
  QString m_fileUri;
  int m_documentVersion = 0;
  bool m_suppressDocumentSync = false;
  QString m_documentText;
  QList<LspTextChange> m_pendingDocumentChanges;
  QTimer *m_documentSyncTimer = nullptr;

  QList<LspDiagnostic> m_diagnostics;
  QList<QTextEdit::ExtraSelection> m_diagnosticSelections;
  QList<QTextEdit::ExtraSelection> m_bracketSelections;
  QList<QTextEdit::ExtraSelection> m_lspHighlightSelections;
  QList<QTextEdit::ExtraSelection> m_findSelections;
  QTimer *m_hoverTimer = nullptr;
  QTimer *m_documentHighlightTimer = nullptr;
  int m_hoverLine = -1;
  int m_hoverChar = -1;
  int m_hoverTimerId = 0;

  QColor m_editorBg;
  QColor m_editorFg;
  QColor m_editorSelection;
  QColor m_editorCurrentLine;
  QColor m_editorLineNumber;
  QColor m_gutterBg;
  QColor m_gutterActive;
  QColor m_border;
  QColor m_bracketBg;
  QColor m_bracketFg;
  VimMotionController *m_vimController = nullptr;
};

class LineNumberArea : public QWidget {
public:
  explicit LineNumberArea(CodeEditor *editor)
      : QWidget(editor), codeEditor(editor) {}

  QSize sizeHint() const override {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

private:
  CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
