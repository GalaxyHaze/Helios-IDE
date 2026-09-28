#include "SidebarController.h"

#include "../panels/GitPanel.h"

#include <QStackedWidget>

#include <utility>

SidebarController::SidebarController(
    ActivityBar *activityBar, QStackedWidget *sidePanel, GitPanel *gitPanel,
    PersistVisibility persistVisibility, PrepareSettings prepareSettings,
    FocusExplorer focusExplorer,
    QObject *parent)
    : QObject(parent),
      m_activityBar(activityBar),
      m_sidePanel(sidePanel),
      m_gitPanel(gitPanel),
      m_persistVisibility(std::move(persistVisibility)),
      m_prepareSettings(std::move(prepareSettings)),
      m_focusExplorer(std::move(focusExplorer))
{
    if (m_activityBar) {
        connect(m_activityBar, &ActivityBar::modeChanged, this,
                &SidebarController::selectMode);
    }
}

void SidebarController::setVisible(bool visible)
{
    if (!m_sidePanel)
        return;

    m_sidePanel->setVisible(visible);
    if (m_activityBar) {
        const int currentIndex = m_sidePanel->currentIndex();
        const int maxMode = static_cast<int>(ActivityBar::Settings);
        const int boundedIndex = qBound(0, currentIndex, maxMode);
        m_activityBar->setActiveMode(
            static_cast<ActivityBar::Mode>(boundedIndex), visible);
    }
    if (m_persistVisibility)
        m_persistVisibility(visible);
}

void SidebarController::selectMode(ActivityBar::Mode mode)
{
    if (!m_sidePanel)
        return;

    if (m_sidePanel->isVisible() &&
        m_sidePanel->currentIndex() == static_cast<int>(mode)) {
        setVisible(false);
        return;
    }

    m_sidePanel->setCurrentIndex(static_cast<int>(mode));
    if (mode == ActivityBar::Git && m_gitPanel)
        m_gitPanel->refreshStatus();
    setVisible(true);
}

void SidebarController::showSettings()
{
    if (!m_sidePanel)
        return;

    if (m_prepareSettings)
        m_prepareSettings();
    m_sidePanel->setCurrentIndex(static_cast<int>(ActivityBar::Settings));
    setVisible(true);
}

void SidebarController::synchronizeActivityBar()
{
    if (!m_sidePanel || !m_activityBar)
        return;

    const int maxMode = static_cast<int>(ActivityBar::Settings);
    const int boundedIndex = qBound(0, m_sidePanel->currentIndex(), maxMode);
    m_activityBar->setActiveMode(
        static_cast<ActivityBar::Mode>(boundedIndex), m_sidePanel->isVisible());
}

bool SidebarController::handleShellCommand(ShellCommand command)
{
    switch (command) {
    case ShellCommand::Explorer:
        selectMode(ActivityBar::Explorer);
        if (m_sidePanel && m_sidePanel->isVisible() && m_focusExplorer)
            m_focusExplorer();
        return true;
    case ShellCommand::WorkspaceSearch:
        selectMode(ActivityBar::Search);
        return true;
    case ShellCommand::Git:
        selectMode(ActivityBar::Git);
        return true;
    case ShellCommand::Settings:
        showSettings();
        return true;
    case ShellCommand::HideSidebar:
        setVisible(false);
        return true;
    default:
        return false;
    }
}
