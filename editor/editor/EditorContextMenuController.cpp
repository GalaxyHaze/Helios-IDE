#include "EditorContextMenuController.h"

#include "Code.h"
#include "EditorLanguageFeatureController.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QTextCursor>
#include <functional>
#include <memory>

namespace
{
QAction *addFeatureAction(
    QMenu *menu, const QString &label, CodeEditor *editor,
    EditorLanguageFeatureController *features,
    EditorLanguageFeatureController::Feature feature,
    const std::function<void()> &action)
{
    QAction *result = menu->addAction(label, editor, action);
    const bool available = features && features->isAvailable(feature);
    result->setEnabled(available);
    if (!available) {
        auto &translation = TranslationManager::instance();
        result->setToolTip(
            translation.translate("stub.disabled_reason").arg(label));
    }
    return result;
}
}

EditorContextMenuController::EditorContextMenuController(
    CodeEditor *editor, EditorLanguageFeatureController *features,
    QObject *parent)
    : QObject(parent), m_editor(editor), m_features(features)
{
}

std::unique_ptr<QMenu> EditorContextMenuController::createMenu()
{
    if (!m_editor)
        return {};

    auto &translation = TranslationManager::instance();
    auto menu = std::make_unique<QMenu>();

    addFeatureAction(
        menu.get(), translation.translate("menu.go_definition"), m_editor,
        m_features, EditorLanguageFeatureController::Feature::Definition,
        [this]() { m_features->requestDefinition(); });
    addFeatureAction(
        menu.get(), translation.translate("menu.go_declaration"), m_editor,
        m_features, EditorLanguageFeatureController::Feature::Declaration,
        [this]() { m_features->requestDeclaration(); });
    addFeatureAction(
        menu.get(), translation.translate("menu.go_implementation"), m_editor,
        m_features, EditorLanguageFeatureController::Feature::Implementation,
        [this]() { m_features->requestImplementation(); });
    addFeatureAction(
        menu.get(), translation.translate("menu.find_usages"), m_editor,
        m_features,
        EditorLanguageFeatureController::Feature::References,
        [this]() { m_features->requestReferences(); });

    menu->addSeparator();

    addFeatureAction(
        menu.get(), translation.translate("menu.rename_symbol"), m_editor,
        m_features, EditorLanguageFeatureController::Feature::Rename,
        [this]() {
            bool accepted = false;
            const QString name = QInputDialog::getText(
                m_editor, "Rename Symbol", "New name:", QLineEdit::Normal, {},
                &accepted);
            if (accepted && !name.trimmed().isEmpty())
                m_features->requestRename(name);
        });
    addFeatureAction(
        menu.get(), translation.translate("menu.extract_method"), m_editor,
        m_features, EditorLanguageFeatureController::Feature::CodeActions,
        [this]() { m_features->requestCodeActions(); });

    menu->addSeparator();

    addFeatureAction(
        menu.get(), translation.translate("menu.format_doc"), m_editor,
        m_features,
        EditorLanguageFeatureController::Feature::Formatting,
        [this]() { m_features->requestFormatting(); });

    menu->addAction(translation.translate("menu.copy_symbol"), m_editor,
                    [this]() {
                        QTextCursor cursor = m_editor->textCursor();
                        cursor.select(QTextCursor::WordUnderCursor);
                        const QString word = cursor.selectedText();
                        if (!word.isEmpty())
                            QApplication::clipboard()->setText(word);
                    });

    auto &theme = ThemeManager::instance();
    menu->setStyleSheet(
        QStringLiteral(
            "QMenu { background: %1; color: %2; border: 1px solid %3; }"
            "QMenu::item:selected { background: %4; color: %5; }"
            "QMenu::item:disabled { color: #6c7086; }")
            .arg(theme.palette().color(QPalette::Base).name(),
                 theme.palette().color(QPalette::Text).name(),
                 theme.customColor("sidebarBorder", QColor("#363a4f")).name(),
                 theme.palette().color(QPalette::Highlight).name(),
                 theme.palette().color(QPalette::HighlightedText).name()));

    return menu;
}

void EditorContextMenuController::show(const QPoint &globalPosition)
{
    auto menu = createMenu();
    if (menu)
        menu->exec(globalPosition);
}
