#include "EditorFileController.h"

#include "../editor/Code.h"

#include <QFileDialog>
#include <QTabWidget>
#include <QWidget>

#include <utility>

EditorFileController::EditorFileController(
    QWidget *dialogParent, QTabWidget *tabWidget,
    EditorSessionController *session, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_dialogParent(dialogParent),
      m_tabWidget(tabWidget),
      m_session(session),
      m_callbacks(std::move(callbacks))
{
}

bool EditorFileController::handleShellCommand(ShellCommand command)
{
    switch (command) {
    case ShellCommand::NewFile:
        newFile();
        return true;
    case ShellCommand::OpenFile:
        openFile();
        return true;
    case ShellCommand::SaveFile:
        saveEditor(currentEditor());
        return true;
    default:
        return false;
    }
}

void EditorFileController::newFile()
{
    if (m_session)
        m_session->createTab(true);
}

void EditorFileController::openFile()
{
    const QString initialPath =
        m_callbacks.workspaceRoot ? m_callbacks.workspaceRoot() : QString();
    const QString path = QFileDialog::getOpenFileName(
        m_dialogParent, QStringLiteral("Open file"), initialPath,
        QStringLiteral("Zith Files (*.zith);;All Files (*)"));
    if (!path.isEmpty() && m_session)
        m_session->openFilePath(path);
}

bool EditorFileController::saveEditor(CodeEditor *editor)
{
    if (!editor || !m_session)
        return false;

    if (m_tabWidget)
        m_tabWidget->setCurrentWidget(editor);

    QString targetPath;
    if (editor->filePath().isEmpty()) {
        const QString initialPath =
            m_callbacks.workspaceRoot ? m_callbacks.workspaceRoot() : QString();
        targetPath = QFileDialog::getSaveFileName(
            m_dialogParent, QStringLiteral("Save File As"), initialPath,
            QStringLiteral("Zith Files (*.zith);;All Files (*)"));
        if (targetPath.isEmpty())
            return false;
    }

    const auto result = m_session->saveEditor(editor, targetPath);
    switch (result) {
    case EditorSessionController::SaveResult::Saved:
        showStatus(QStringLiteral("File saved"), 2000);
        if (m_callbacks.refreshCommandAvailability)
            m_callbacks.refreshCommandAvailability();
        return true;
    case EditorSessionController::SaveResult::WriteFailed:
        showStatus(QStringLiteral("Could not save file"), 5000);
        return false;
    case EditorSessionController::SaveResult::MissingPath:
        return false;
    }

    return false;
}

CodeEditor *EditorFileController::currentEditor() const
{
    return m_tabWidget
               ? qobject_cast<CodeEditor *>(m_tabWidget->currentWidget())
               : nullptr;
}

void EditorFileController::showStatus(const QString &message, int timeout) const
{
    if (m_callbacks.showStatus)
        m_callbacks.showStatus(message, timeout);
}
