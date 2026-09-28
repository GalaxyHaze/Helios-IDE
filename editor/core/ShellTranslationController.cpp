#include "ShellTranslationController.h"

#include "ShellCommandSurface.h"
#include "TranslationManager.h"
#include "../widgets/ActivityBar.h"

ShellTranslationController::ShellTranslationController(
    ActivityBar *activityBar, ShellCommandSurface *commandSurface,
    QObject *parent)
    : QObject(parent), m_activityBar(activityBar),
      m_commandSurface(commandSurface)
{
}

void ShellTranslationController::apply()
{
    const auto &translation = TranslationManager::instance();

    if (m_commandSurface)
        m_commandSurface->applyTranslations();
    if (!m_activityBar)
        return;

    m_activityBar->setButtonToolTip(
        ActivityBar::Explorer,
        translation.translate(QStringLiteral("shortcut.explorer")) +
            QStringLiteral(" (Ctrl+Shift+E)"));
    m_activityBar->setButtonToolTip(
        ActivityBar::Search,
        translation.translate(QStringLiteral("shortcut.global_search")) +
            QStringLiteral(" (Ctrl+Shift+F)"));
    m_activityBar->setButtonToolTip(
        ActivityBar::Git,
        translation.translate(QStringLiteral("shortcut.git_panel")) +
            QStringLiteral(" (Ctrl+Shift+G)"));
    m_activityBar->setButtonToolTip(
        ActivityBar::Settings,
        translation.translate(QStringLiteral("shortcut.settings")) +
            QStringLiteral(" (Ctrl+,)"));
}
