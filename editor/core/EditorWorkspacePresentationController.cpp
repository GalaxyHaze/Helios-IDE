#include "EditorWorkspacePresentationController.h"

#include "../editor/Code.h"
#include "../widgets/BreadcrumbsBar.h"
#include "../widgets/FindReplaceBar.h"
#include "../panels/WelcomeWidget.h"

#include <QStackedWidget>
#include <QTabWidget>

EditorWorkspacePresentationController::EditorWorkspacePresentationController(
    Dependencies dependencies, QObject *parent)
    : QObject(parent),
      m_tabs(dependencies.tabs),
      m_centralStack(dependencies.centralStack),
      m_welcomeWidget(dependencies.welcomeWidget),
      m_editorPanel(dependencies.editorPanel),
      m_breadcrumbs(dependencies.breadcrumbs),
      m_findReplaceBar(dependencies.findReplaceBar)
{
    if (m_tabs) {
        connect(m_tabs, &QTabWidget::currentChanged, this,
                &EditorWorkspacePresentationController::synchronize);
    }
}

void EditorWorkspacePresentationController::synchronize()
{
    if (!m_tabs || !m_centralStack)
        return;

    if (m_tabs->count() == 0) {
        if (m_welcomeWidget)
            m_centralStack->setCurrentWidget(m_welcomeWidget);
        if (m_breadcrumbs)
            m_breadcrumbs->hide();
        if (m_findReplaceBar)
            m_findReplaceBar->hide();
        return;
    }

    if (m_editorPanel)
        m_centralStack->setCurrentWidget(m_editorPanel);
    if (m_breadcrumbs)
        m_breadcrumbs->show();
}

void EditorWorkspacePresentationController::prepareFindBar()
{
    if (!m_findReplaceBar)
        return;

    m_findReplaceBar->setEditor(
        m_tabs ? qobject_cast<CodeEditor *>(m_tabs->currentWidget()) : nullptr);
}

void EditorWorkspacePresentationController::showFind()
{
    prepareFindBar();
    if (m_findReplaceBar)
        m_findReplaceBar->showFind();
}

void EditorWorkspacePresentationController::showReplace()
{
    prepareFindBar();
    if (m_findReplaceBar)
        m_findReplaceBar->showReplace();
}

void EditorWorkspacePresentationController::findNext()
{
    if (!m_findReplaceBar)
        return;

    if (m_findReplaceBar->isVisible())
        m_findReplaceBar->findNext();
    else
        showFind();
}

void EditorWorkspacePresentationController::findPrevious()
{
    if (!m_findReplaceBar)
        return;

    if (m_findReplaceBar->isVisible())
        m_findReplaceBar->findPrevious();
    else
        showFind();
}

bool EditorWorkspacePresentationController::handleShellCommand(
    ShellCommand command)
{
    switch (command) {
    case ShellCommand::Find:
        showFind();
        return true;
    case ShellCommand::Replace:
        showReplace();
        return true;
    case ShellCommand::FindNext:
        findNext();
        return true;
    case ShellCommand::FindPrevious:
        findPrevious();
        return true;
    default:
        return false;
    }
}
