#include "WorkspacePanelPresentationController.h"

#include "../panels/BottomPanel.h"
#include "../panels/OutlinePanel.h"
#include "EditorChromeController.h"
#include "ShellCommandSurface.h"

#include <utility>

WorkspacePanelPresentationController::WorkspacePanelPresentationController(
    Dependencies dependencies, CurrentEditor currentEditor,
    PersistOutlineVisibility persistOutlineVisibility, QObject *parent)
    : QObject(parent),
      m_outlinePanel(dependencies.outlinePanel),
      m_bottomPanel(dependencies.bottomPanel),
      m_commandSurface(dependencies.commandSurface),
      m_editorChrome(dependencies.editorChrome),
      m_currentEditor(std::move(currentEditor)),
      m_persistOutlineVisibility(std::move(persistOutlineVisibility))
{
    if (m_bottomPanel) {
        connect(m_bottomPanel, &BottomPanel::closeRequested, this,
                [this]() { setBottomPanelVisible(false); });
        connect(m_bottomPanel, &BottomPanel::visibilityChanged, this,
                [this](bool) { synchronize(); });
    }
    synchronize();
}

void WorkspacePanelPresentationController::setOutlineVisible(bool visible)
{
    if (!m_outlinePanel)
        return;

    m_outlinePanel->setVisible(visible);
    if (m_persistOutlineVisibility)
        m_persistOutlineVisibility(visible);

    if (!visible) {
        m_outlinePanel->clear();
    } else if (m_currentEditor && m_editorChrome) {
        m_editorChrome->update(m_currentEditor());
    }
    synchronize();
}

void WorkspacePanelPresentationController::toggleOutline()
{
    if (m_outlinePanel)
        setOutlineVisible(!m_outlinePanel->isVisible());
}

void WorkspacePanelPresentationController::setBottomPanelVisible(bool visible)
{
    if (!m_bottomPanel)
        return;

    m_bottomPanel->setVisible(visible);
    synchronize();
}

void WorkspacePanelPresentationController::toggleBottomPanel()
{
    if (m_bottomPanel)
        setBottomPanelVisible(!m_bottomPanel->isVisible());
}

bool WorkspacePanelPresentationController::handleShellCommand(
    ShellCommand command)
{
    switch (command) {
    case ShellCommand::ToggleOutline:
        toggleOutline();
        return true;
    case ShellCommand::ToggleBottomPanel:
        toggleBottomPanel();
        return true;
    default:
        return false;
    }
}

void WorkspacePanelPresentationController::synchronize()
{
    if (!m_commandSurface)
        return;

    if (m_outlinePanel)
        m_commandSurface->setOutlineVisible(m_outlinePanel->isVisible());
    if (m_bottomPanel)
        m_commandSurface->setBottomPanelVisible(m_bottomPanel->isVisible());
}
