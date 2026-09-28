#include "EditorAppearanceController.h"

#include "../core/ThemeManager.h"

#include <QPalette>
#include <QPlainTextEdit>

EditorAppearanceController::EditorAppearanceController(
    QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            &EditorAppearanceController::apply);
}

void EditorAppearanceController::apply()
{
    auto &theme = ThemeManager::instance();
    const QPalette palette = theme.palette();

    m_appearance.background =
        theme.customColor("editorBg", palette.color(QPalette::Base));
    m_appearance.foreground =
        theme.customColor("editorFg", palette.color(QPalette::Text));
    m_appearance.selection =
        theme.customColor("editorSelection", palette.color(QPalette::Highlight));
    m_appearance.currentLine = theme.customColor(
        "editorCurrentLine", palette.color(QPalette::AlternateBase));
    m_appearance.lineNumber = theme.customColor(
        "editorLineNumber", palette.color(QPalette::PlaceholderText));
    m_appearance.gutterBackground =
        theme.customColor("gutterBg", palette.color(QPalette::Window));
    m_appearance.gutterActive =
        theme.customColor("gutterActive", palette.color(QPalette::Link));
    m_appearance.border =
        theme.customColor("sidebarBorder", palette.color(QPalette::Shadow));
    m_appearance.bracketBackground =
        theme.customColor("bracketBg", palette.color(QPalette::Highlight));
    m_appearance.bracketForeground = theme.customColor(
        "bracketFg", palette.color(QPalette::HighlightedText));
    m_appearance.diagnosticError =
        theme.customColor("diagnosticError", QColor("#ff7a90"));
    m_appearance.diagnosticWarning =
        theme.customColor("diagnosticWarning", QColor("#f1c77a"));
    m_appearance.diagnosticInfo =
        theme.customColor("diagnosticInfo", QColor("#70c9f0"));
    m_appearance.diagnosticUnknown =
        theme.customColor("diagnosticUnknown", QColor("#aeb8ce"));

    if (m_editor) {
        QPalette editorPalette = m_editor->palette();
        editorPalette.setColor(QPalette::Base, m_appearance.background);
        editorPalette.setColor(QPalette::Text, m_appearance.foreground);
        editorPalette.setColor(QPalette::Highlight, m_appearance.selection);
        editorPalette.setColor(QPalette::HighlightedText,
                               m_appearance.foreground);
        m_editor->setPalette(editorPalette);

        const QString styleSheet =
            QStringLiteral(
                "QPlainTextEdit { background-color: %1; color: %2; "
                "selection-background-color: %3; }"
                "QPlainTextEdit:focus { border: none; }")
                .arg(m_appearance.background.name(),
                     m_appearance.foreground.name(),
                     m_appearance.selection.name());
        if (m_editor->styleSheet() != styleSheet)
            m_editor->setStyleSheet(styleSheet);
    }

    emit appearanceChanged();
}
