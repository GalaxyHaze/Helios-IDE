#include "ApplicationThemeController.h"

#include "ApplicationStyle.h"
#include "AppearanceController.h"
#include "StatusBarController.h"
#include "ThemeManager.h"
#include "../widgets/BreadcrumbsBar.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QMenuBar>
#include <QSplitter>
#include <QTabWidget>
#include <QWidget>

namespace {
void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet)
{
    if (widget && widget->styleSheet() != styleSheet)
        widget->setStyleSheet(styleSheet);
}
}

ApplicationThemeController::ApplicationThemeController(
    QWidget *window, QTabWidget *tabWidget, QSplitter *splitter,
    BreadcrumbsBar *breadcrumbs, QMenuBar *menuBar,
    StatusBarController *statusBar, QObject *parent)
    : QObject(parent), m_window(window), m_tabWidget(tabWidget),
      m_splitter(splitter), m_breadcrumbs(breadcrumbs),
      m_menuBar(menuBar), m_statusBar(statusBar)
{
}

void ApplicationThemeController::apply()
{
#ifdef HELIOS_THEME_TIMING
    QElapsedTimer timer;
    timer.start();
#endif

    auto &theme = ThemeManager::instance();
    auto &appearance = AppearanceController::instance();
    QApplication::setPalette(theme.palette());
    if (m_menuBar)
        m_menuBar->setFont(appearance.uiFont());
    setStyleSheetIfChanged(m_window, ApplicationStyle::globalStyleSheet());

    if (m_statusBar)
        m_statusBar->applyTheme();

    setStyleSheetIfChanged(
        m_tabWidget,
        QStringLiteral(
            "QTabWidget::pane { border: none; background: %1; }"
            "QTabBar { background: %1; padding-top: 4px; }"
            "QTabBar::tab { background: %1; color: %2; padding: 6px 14px;"
            "  border-right: 1px solid %3; border-radius: 6px 6px 0 0;"
            "  font-size: %7; margin-left: 2px; }"
            "QTabBar::tab:selected { color: %4; background: %5;"
            "  border-top: 2px solid %8; }"
            "QTabBar::tab:hover:!selected { background: %6; }")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::SurfaceMuted)
                     .name(),
                 theme.semanticColor(ThemeManager::SemanticRole::TextMuted)
                     .name(),
                 theme
                     .semanticColor(ThemeManager::SemanticRole::BorderStrong)
                     .name(),
                 theme
                     .semanticColor(ThemeManager::SemanticRole::SelectedText)
                     .name(),
                 theme.semanticColor(ThemeManager::SemanticRole::SurfaceAlt)
                     .name(),
                 theme.semanticColor(ThemeManager::SemanticRole::Hover).name(),
                 QString::number(qMax(appearance.uiFont().pointSize() - 2,
                                      appearance.minFontSize())),
                 theme.semanticColor(ThemeManager::SemanticRole::Accent)
                     .name()));

    setStyleSheetIfChanged(
        m_splitter,
        QStringLiteral(
            "QSplitter::handle { background: transparent; width: 3px; "
            "margin: 0 0 0 -1px; }"
            "QSplitter::handle:hover { background: %1; }")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::AccentHover)
                     .name()));

    setStyleSheetIfChanged(
        m_breadcrumbs,
        QStringLiteral(
            "BreadcrumbsBar { background: %1; color: %2;"
            " border-radius: 6px 6px 0 0; padding: 2px 0; }")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::SurfaceAlt)
                     .name(),
                 theme.semanticColor(ThemeManager::SemanticRole::TextMuted)
                     .name()));

#ifdef HELIOS_THEME_TIMING
    qDebug() << "Theme application completed in" << timer.elapsed() << "ms";
#endif
}
