#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "EditorLanguageRequestContext.h"
#include "EditorDocumentSyncController.h"
#include "LspTypes.h"
#include <QHash>
#include <QList>
#include <QMap>
#include <QPlainTextEdit>
#include <QWidget>

class LspCompleter;
class SnippetManager;
class LineNumberArea;
class VimMotionController;
class EditorLanguageFeatureController;
class EditorContextMenuController;
class EditorAppearanceController;
class EditorDecorationController;
struct EditorAppearance;
class LspClient;

class CodeEditor : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit CodeEditor(QWidget *parent = nullptr);
  ~CodeEditor() override;

  void setFilePath(const QString &path);
  void setInitialDocumentText(const QString &text, int initialVersion = 1);
  QString filePath() const { return m_filePath; }
  QString fileUri() const { return m_fileUri; }
  LspClient *lspClient() const { return m_lspClient; }
  int documentVersion() const { return m_documentSyncController.version(); }
  QList<LspDiagnostic> diagnostics() const { return m_diagnostics; }
  void flushPendingLspChanges() { flushDocumentChanges(); }
  EditorLanguageRequestContext currentLanguageRequest();
  bool prepareCompletion();
  bool handleCompletionKey(QKeyEvent *event);

  void setLspClient(LspClient *client);
  void detachLspClient();
  void markLspDocumentSynchronized();
  void setCompleter(LspCompleter *completer);
  void setSnippetManager(SnippetManager *manager);

  void setDiagnostics(const QList<LspDiagnostic> &diagnostics);
  void clearDiagnostics();
  void setSemanticTokens(const QList<LspSemanticToken> &tokens);
  void clearSemanticTokens();
  void setFoldingRanges(const QList<LspFoldingRange> &ranges);
  void clearFoldingRanges();
  void setLspHighlightRanges(const QList<LspRange> &ranges);

  void updateDiagnosticDisplay();

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  void toggleFoldAtLine(int line);
  bool hasFoldingRangeAtLine(int line) const;
  bool isFoldedAtLine(int line) const;
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
  void vimCommandEntered(const QString &command);

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
  void onDocumentContentsChanged(int position, int charsRemoved,
                                 int charsAdded);
  void flushDocumentChanges();
  void updateTheme();

private:
  void triggerCompletion();
  void applyCompletionItem(const LspCompletionItem &item);
  void triggerSignatureHelp();
  void autoIndent();
  bool handleAutoClose(QChar ch);
  bool tryExpandSnippet();
  const EditorAppearance &editorAppearance() const;
  LineNumberArea *lineNumberArea;
  LspClient *m_lspClient = nullptr;
  LspCompleter *m_completer = nullptr;
  SnippetManager *m_snippetManager = nullptr;

  QString m_filePath;
  QString m_fileUri;
  bool m_suppressDocumentSync = false;

  QList<LspDiagnostic> m_diagnostics;
  QList<LspFoldingRange> m_foldingRanges;
  QHash<int, int> m_hiddenLineCounts;
  EditorAppearanceController *m_appearanceController = nullptr;
  EditorDecorationController *m_decorationController = nullptr;
  VimMotionController *m_vimController = nullptr;
  EditorLanguageFeatureController *m_languageFeatures = nullptr;
  EditorContextMenuController *m_contextMenuController = nullptr;
  EditorDocumentSyncController m_documentSyncController;
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
  void mousePressEvent(QMouseEvent *event) override;

private:
  CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
