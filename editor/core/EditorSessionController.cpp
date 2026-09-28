#include "EditorSessionController.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "../editor/LspCompletionModel.h"
#include "../editor/Syntax.h"
#include "../panels/DiagnosticsPanel.h"
#include "AppearanceController.h"
#include "EditorChromeController.h"
#include "EditorSyntaxController.h"
#include "FileIcons.h"
#include "LanguageIdentity.h"
#include "LspDocumentCoordinator.h"
#include "LspLogPresenter.h"
#include "SnippetManager.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>

EditorSessionController::EditorSessionController(
    Dependencies dependencies, Callbacks callbacks,
    QObject *parent)
    : QObject(parent),
      m_tabWidget(dependencies.tabWidget),
      m_snippetManager(dependencies.snippetManager),
      m_completer(dependencies.completer),
      m_completionModel(dependencies.completionModel),
      m_diagnosticsPanel(dependencies.diagnosticsPanel),
      m_syntaxController(dependencies.syntaxController),
      m_documentCoordinator(dependencies.documentCoordinator),
      m_logPresenter(dependencies.logPresenter),
      m_editorChrome(dependencies.editorChrome),
      m_callbacks(std::move(callbacks))
{
    m_editorPreferences.editorFont =
        AppearanceController::instance().editorFont();
}

CodeEditor *EditorSessionController::createTab(bool makeCurrent)
{
    auto *editor = new CodeEditor;
    editor->setFont(m_editorPreferences.editorFont);
    editor->setLineWrapMode(
        m_editorPreferences.wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                             : QPlainTextEdit::NoWrap);
    editor->setSnippetManager(m_snippetManager);
    editor->setCompleter(m_completer);
    editor->setLspClient(nullptr);
    editor->setVimMotionsEnabled(m_editorPreferences.vimMotionsEnabled);

    if (m_callbacks.connectEditorSignals)
        m_callbacks.connectEditorSignals(editor);

    int index = -1;
    if (makeCurrent) {
        index = m_tabWidget->addTab(editor, QStringLiteral("Untitled"));
        m_tabWidget->setCurrentIndex(index);
    } else {
        QSignalBlocker blocker(m_tabWidget);
        index = m_tabWidget->addTab(editor, QStringLiteral("Untitled"));
    }
    m_tabWidget->setTabToolTip(index, {});
    m_tabWidget->setTabIcon(index, fileIconForSuffix({}));

    if (m_callbacks.updateCentralWidgetState)
        m_callbacks.updateCentralWidgetState();
    return editor;
}

void EditorSessionController::setEditorPreferences(
    const EditorPreferences &preferences)
{
    m_editorPreferences = preferences;
    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (!editor)
            continue;

        editor->setFont(m_editorPreferences.editorFont);
        editor->setLineWrapMode(
            m_editorPreferences.wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                                 : QPlainTextEdit::NoWrap);
        editor->setVimMotionsEnabled(m_editorPreferences.vimMotionsEnabled);
        editor->update();
    }
}

void EditorSessionController::updateTabMetadata(CodeEditor *editor,
                                                const QString &path)
{
    const int index = m_tabWidget->indexOf(editor);
    if (index < 0)
        return;
    m_tabWidget->setTabText(index, QFileInfo(path).fileName());
    m_tabWidget->setTabToolTip(index, path);
    m_tabWidget->setTabIcon(index, fileIconForPath(path));
}

bool EditorSessionController::openFilePath(const QString &path)
{
    if (path.isEmpty())
        return false;

    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (editor && editor->filePath() == path) {
            m_tabWidget->setCurrentIndex(index);
            if (m_editorChrome)
                m_editorChrome->update(editor);
            if (m_callbacks.updateCentralWidgetState)
                m_callbacks.updateCentralWidgetState();
            return true;
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    auto *editor = createTab(false);
    if (!assignPath(editor, path)) {
        releaseEditor(editor);
        return false;
    }
    editor->setInitialDocumentText(content);
    editor->clearDiagnostics();

    if (m_diagnosticsPanel)
        m_diagnosticsPanel->clearDiagnostics(editor->fileUri());
    if (m_completionModel)
        m_completionModel->setItems({});

    const int index = m_tabWidget->indexOf(editor);
    m_tabWidget->setCurrentIndex(index);
    if (m_editorChrome)
        m_editorChrome->update(editor);
    if (m_callbacks.updateCentralWidgetState)
        m_callbacks.updateCentralWidgetState();

    QTimer::singleShot(0, this, [this, editor]() {
        openDocument(editor);
    });
    return true;
}

bool EditorSessionController::assignPath(CodeEditor *editor,
                                         const QString &path)
{
    if (!editor || path.isEmpty())
        return false;

    if (m_documentCoordinator)
        m_documentCoordinator->close(editor);
    if (m_diagnosticsPanel)
        m_diagnosticsPanel->clearDiagnostics(editor->fileUri());
    if (m_completionModel)
        m_completionModel->setItems({});

    editor->setFilePath(path);
    editor->setLspClient(nullptr);
    if (m_syntaxController)
        m_syntaxController->apply(editor, LanguageIdentity::forPath(path));
    updateTabMetadata(editor, path);
    if (m_editorChrome)
        m_editorChrome->update(editor);
    publishDocumentSetChanged();
    return true;
}

EditorSessionController::SaveResult
EditorSessionController::saveEditor(CodeEditor *editor, const QString &path)
{
    if (!editor)
        return SaveResult::MissingPath;

    const bool wasUntitled = editor->filePath().isEmpty();
    const QString targetPath = wasUntitled ? path : editor->filePath();
    if (targetPath.isEmpty())
        return SaveResult::MissingPath;

    QFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return SaveResult::WriteFailed;

    QTextStream stream(&file);
    stream << editor->toPlainText();
    file.close();

    if (wasUntitled && !assignPath(editor, targetPath))
        return SaveResult::WriteFailed;

    editor->document()->setModified(false);
    saveDocument(editor);
    return SaveResult::Saved;
}

void EditorSessionController::releaseEditor(CodeEditor *editor)
{
    if (!editor)
        return;

    if (m_documentCoordinator)
        m_documentCoordinator->close(editor);
    if (m_diagnosticsPanel)
        m_diagnosticsPanel->clearDiagnostics(editor->fileUri());

    const int index = m_tabWidget->indexOf(editor);
    if (index >= 0) {
        m_tabWidget->removeTab(index);
        publishDocumentSetChanged();
    } else {
        return;
    }
    if (m_syntaxController)
        m_syntaxController->remove(editor);
    editor->deleteLater();
}

void EditorSessionController::openDocument(CodeEditor *editor)
{
    if (!editor || !m_documentCoordinator)
        return;
    if (m_callbacks.refreshLspRouting)
        m_callbacks.refreshLspRouting();
    m_documentCoordinator->openDocument(editor);
}

void EditorSessionController::saveDocument(CodeEditor *editor)
{
    if (!editor || !m_documentCoordinator)
        return;
    if (m_callbacks.refreshLspRouting)
        m_callbacks.refreshLspRouting();
    m_documentCoordinator->saveDocument(editor);
}

void EditorSessionController::appendLspLog(const QString &message) const
{
    if (m_logPresenter)
        m_logPresenter->append(message);
}

void EditorSessionController::saveAllForLsp()
{
    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (!editor)
            continue;

        editor->flushPendingLspChanges();
        if (editor->filePath().isEmpty()) {
            if (editor->document()->isModified())
                appendLspLog(QStringLiteral(
                    "Cannot save untitled document requested by "
                    "zith/requestSaveAll."));
            continue;
        }
        if (!editor->document()->isModified())
            continue;

        const SaveResult result = saveEditor(editor);
        if (result != SaveResult::Saved) {
            appendLspLog(QStringLiteral("Could not save ") +
                         editor->filePath());
            continue;
        }
    }
}

EditorSessionState EditorSessionController::captureState() const
{
    EditorSessionState state;
    const int currentWidgetIndex = m_tabWidget->currentIndex();
    int persistedTabIndex = 0;
    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (editor && !editor->filePath().isEmpty()) {
            state.openFiles.append(editor->filePath());
            if (index == currentWidgetIndex)
                state.currentTab = persistedTabIndex;
            ++persistedTabIndex;
        }
    }
    return state;
}

void EditorSessionController::restoreState(const EditorSessionState &state)
{
    m_batchingDocumentSetChanges = true;
    m_documentSetChangedWhileBatching = false;

    while (m_tabWidget->count() > 0) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(0));
        if (editor)
            releaseEditor(editor);
    }

    int firstRestoredTab = -1;
    int requestedActiveTab = -1;
    for (int persistedIndex = 0; persistedIndex < state.openFiles.size();
         ++persistedIndex) {
        if (!openFilePath(state.openFiles.at(persistedIndex)))
            continue;

        const int restoredTab = m_tabWidget->currentIndex();
        if (firstRestoredTab < 0)
            firstRestoredTab = restoredTab;
        if (persistedIndex == state.currentTab)
            requestedActiveTab = restoredTab;
    }

    if (requestedActiveTab >= 0)
        m_tabWidget->setCurrentIndex(requestedActiveTab);
    else if (firstRestoredTab >= 0)
        m_tabWidget->setCurrentIndex(firstRestoredTab);
    if (m_callbacks.updateCentralWidgetState)
        m_callbacks.updateCentralWidgetState();

    m_batchingDocumentSetChanges = false;
    if (m_documentSetChangedWhileBatching) {
        m_documentSetChangedWhileBatching = false;
        emit documentSetChanged();
    }
}

void EditorSessionController::publishDocumentSetChanged()
{
    if (m_batchingDocumentSetChanges) {
        m_documentSetChangedWhileBatching = true;
        return;
    }

    emit documentSetChanged();
}
