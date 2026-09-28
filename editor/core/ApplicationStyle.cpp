#include "ApplicationStyle.h"

#include "ThemeManager.h"

namespace ApplicationStyle
{
QString globalStyleSheet()
{
    auto &theme = ThemeManager::instance();
    const QString background =
        theme.semanticColor(ThemeManager::SemanticRole::Canvas).name();
    const QString text =
        theme.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString base =
        theme.semanticColor(ThemeManager::SemanticRole::SurfaceAlt).name();
    const QString alternateBase = theme.palette().color(QPalette::AlternateBase).name();
    const QString highlight =
        theme.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString border =
        theme.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString hover =
        theme.semanticColor(ThemeManager::SemanticRole::Hover).name();
    const QString faint =
        theme.semanticColor(ThemeManager::SemanticRole::TextFaint).name();
    const QString inputBackground =
        theme.semanticColor(ThemeManager::SemanticRole::InputBg).name();
    const QString buttonBackground =
        theme.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
    const QString accent =
        theme.semanticColor(ThemeManager::SemanticRole::Accent).name();
    const QColor shadowBase =
        theme.isDark() ? QColor(24, 18, 52) : QColor(70, 58, 106);
    const int shadowAlpha = theme.isDark() ? 190 : 125;
    const QString shadow = QStringLiteral("rgba(%1, %2, %3, %4)")
                               .arg(shadowBase.red())
                               .arg(shadowBase.green())
                               .arg(shadowBase.blue())
                               .arg(shadowAlpha);

    return QStringLiteral(
               "QMainWindow, QWidget { background: %1; color: %2; }"
               "QMenuBar { background: %1; color: %2; border-bottom: 1px solid "
               "%3; padding: 2px 4px; }"
               "QMenuBar::item:selected { background: %4; }"
               "QMenuBar::item:pressed { background: %5; }"
               "QMenu { background: %1; color: %2; border: 1px solid %3; "
               "border-left: 2px solid %11; border-bottom: 2px solid %11; "
               "border-radius: 6px; padding: 4px; }"
               "QMenu::item:selected { background: %4; }"
               "QMenu::item:disabled { color: %7; }"
               "QMenu::separator { height: 1px; background: %3; margin: 4px 8px; "
               "}"
               "QStatusBar { background: %1; color: %2; border-top: 1px solid "
               "%3; font-size: 11px; }"
               "QStatusBar::item { border: none; }"
               "QDockWidget { background: %1; color: %2; }"
               "QDockWidget::title { background: %6; color: %2; padding: 4px; }"
               "QScrollBar:vertical { background: transparent; width: 7px; "
               "margin: 2px 3px; }"
               "QScrollBar:vertical:hover { background: transparent; width: 9px; }"
               "QScrollBar::handle:vertical { background: %4; border-radius: 999px; "
               "min-height: 22px; margin: 0 1px; }"
               "QScrollBar::handle:vertical:hover { background: %10; }"
               "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, "
               "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { "
               "width: 0px; height: 0px; background: transparent; }"
               "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, "
               "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { "
               "background: transparent; }"
               "QScrollBar:horizontal { background: transparent; height: 7px; "
               "margin: 3px 2px; }"
               "QScrollBar:horizontal:hover { background: transparent; height: 9px; }"
               "QScrollBar::handle:horizontal { background: %4; border-radius: 999px; "
               "min-width: 22px; margin: 1px 0; }"
               "QPushButton, QToolButton { background: %9; color: %2; border: 1px "
               "solid %3; border-left: 2px solid %11; border-bottom: 2px solid %11; "
               "border-radius: 6px; padding: 6px 12px; }"
               "QPushButton:hover, QToolButton:hover { background: %4; border-left: "
               "3px solid %11; border-bottom: 3px solid %11; }"
               "QPushButton:pressed, QToolButton:pressed { background: %5; "
               "border-left: 1px solid %11; border-bottom: 1px solid %11; }"
               "QLineEdit, QComboBox, QPlainTextEdit, QTextEdit { background: %8; "
               "color: %2; border: 1px solid %3; border-radius: 6px; "
               "padding: 5px 8px; selection-background-color: %5; }"
               "QLineEdit:focus, QComboBox:focus, QPlainTextEdit:focus, "
               "QTextEdit:focus { border: 1px solid %10; }")
        .arg(background, text, border, hover, highlight, alternateBase, faint,
             inputBackground, buttonBackground, accent, shadow);
}
}
