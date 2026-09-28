#include "EditorInteractionController.h"

#include "../editor/Code.h"
#include "AppearanceController.h"
#include "EditorChromeController.h"
#include "EditorLspActionController.h"
#include "LocationNavigator.h"

#include <utility>

EditorInteractionController::EditorInteractionController(
    Dependencies dependencies, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_lspActions(dependencies.lspActions),
      m_editorChrome(dependencies.editorChrome),
      m_locationNavigator(dependencies.locationNavigator),
      m_callbacks(std::move(callbacks)) {}

void EditorInteractionController::attach(CodeEditor *editor) {
  if (!editor)
    return;

  if (m_lspActions)
    m_lspActions->attach(editor);
  if (m_locationNavigator)
    m_locationNavigator->attach(editor);

  connect(editor, &CodeEditor::zoomChanged, this, [](double scale) {
    AppearanceController::instance().setEditorFontSize(qRound(12 * scale));
  });
  connect(editor, &CodeEditor::cursorPositionChanged, this, [this] {
    if (m_callbacks.currentEditor && m_editorChrome) {
      if (auto *current = m_callbacks.currentEditor())
        m_editorChrome->update(current);
    }
  });
  connect(editor, &CodeEditor::vimModeChanged, this,
          [this, editor](const QString &mode) {
            if (!m_callbacks.currentEditor || !m_callbacks.setVimModeLabel)
              return;
            if (editor == m_callbacks.currentEditor())
              m_callbacks.setVimModeLabel("VIM: " + mode);
          });
  connect(editor, &CodeEditor::vimCommandEntered, this,
          [this](const QString &command) { handleVimCommand(command); });
}

void EditorInteractionController::handleVimCommand(const QString &command) {
  const QString trimmed = command.trimmed();
  if (!m_callbacks.currentEditor)
    return;

  CodeEditor *editor = m_callbacks.currentEditor();
  if (!editor)
    return;

  if (trimmed == QStringLiteral("w")) {
    if (m_callbacks.saveCurrent)
      m_callbacks.saveCurrent();
    return;
  }

  if (trimmed == QStringLiteral("q!")) {
    closeCurrentEditor(editor);
    return;
  }

  if (trimmed == QStringLiteral("q")) {
    if (editor->document()->isModified()) {
      showStatus(
          QStringLiteral("No write since last change (use :q! to force)."),
          5000);
      return;
    }
    closeCurrentEditor(editor);
    return;
  }

  if (trimmed == QStringLiteral("wq")) {
    if (m_callbacks.saveCurrent && m_callbacks.saveCurrent() &&
        !editor->document()->isModified())
      closeCurrentEditor(editor);
  }
}

void EditorInteractionController::closeCurrentEditor(CodeEditor *editor) {
  if (m_callbacks.closeEditor)
    m_callbacks.closeEditor(editor);
  if (m_callbacks.updateCentralWidgetState)
    m_callbacks.updateCentralWidgetState();
}

void EditorInteractionController::showStatus(const QString &message,
                                             int timeout) const {
  if (m_callbacks.showStatus)
    m_callbacks.showStatus(message, timeout);
}
