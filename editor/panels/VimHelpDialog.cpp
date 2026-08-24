#include "VimHelpDialog.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>

namespace {
void addShortcutRow(QGridLayout *layout, int row, const QString &keys, const QString &desc) {
    auto *keyLbl = new QLabel(keys);
    auto *descLbl = new QLabel(desc);

    keyLbl->setObjectName(QStringLiteral("vimKeyLabel"));
    layout->addWidget(keyLbl, row, 0, Qt::AlignRight);
    layout->addWidget(descLbl, row, 1, Qt::AlignLeft);
}

void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet) {
    if (widget->styleSheet() != styleSheet)
        widget->setStyleSheet(styleSheet);
}
}

VimHelpDialog::VimHelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(false);
    resize(420, 500);

    auto *mainLayout = new QVBoxLayout(this);
    
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    auto *contentWidget = new QWidget;
    auto *contentLayout = new QVBoxLayout(contentWidget);
    
    auto *title = new QLabel("Vim Motions", this);
    contentLayout->addWidget(title);
    
    auto *hint = new QLabel("Active keyboard motions and commands supported by the Helios Vim extension.", this);
    hint->setWordWrap(true);
    contentLayout->addWidget(hint);
    contentLayout->addSpacing(10);
    
    // Movements
    auto *moveGroup = new QGroupBox("Movements");
    auto *moveLayout = new QGridLayout(moveGroup);
    addShortcutRow(moveLayout, 0, "h j k l", "Character Left/Down/Up/Right");
    addShortcutRow(moveLayout, 1, "w / W", "Next Word (punctuation / whitespace)");
    addShortcutRow(moveLayout, 2, "b / B", "Previous Word (punctuation / whitespace)");
    addShortcutRow(moveLayout, 3, "e", "End of Word");
    addShortcutRow(moveLayout, 4, "0", "Start of Line");
    addShortcutRow(moveLayout, 5, "^", "First non-blank character of Line");
    addShortcutRow(moveLayout, 6, "$", "End of Line");
    contentLayout->addWidget(moveGroup);
    
    // Navigation
    auto *navGroup = new QGroupBox("Navigation");
    auto *navLayout = new QGridLayout(navGroup);
    addShortcutRow(navLayout, 0, "g g", "Go to Start of File");
    addShortcutRow(navLayout, 1, "G", "Go to End of File");
    contentLayout->addWidget(navGroup);
    
    // Search
    auto *searchGroup = new QGroupBox("Search");
    auto *searchLayout = new QGridLayout(searchGroup);
    addShortcutRow(searchLayout, 0, "f <char>", "Find character forward");
    addShortcutRow(searchLayout, 1, "F <char>", "Find character backward");
    addShortcutRow(searchLayout, 2, "t <char>", "Find character forward (till)");
    addShortcutRow(searchLayout, 3, "T <char>", "Find character backward (till)");
    addShortcutRow(searchLayout, 4, ";", "Repeat last find forward");
    addShortcutRow(searchLayout, 5, ",", "Repeat last find backward");
    contentLayout->addWidget(searchGroup);
    
    // Edit & Insert
    auto *editGroup = new QGroupBox("Edit & Insert");
    auto *editLayout = new QGridLayout(editGroup);
    addShortcutRow(editLayout, 0, "i", "Insert before cursor");
    addShortcutRow(editLayout, 1, "a", "Insert after cursor");
    addShortcutRow(editLayout, 2, "I", "Insert at beginning of line");
    addShortcutRow(editLayout, 3, "A", "Insert at end of line");
    addShortcutRow(editLayout, 4, "o", "Open new line below");
    addShortcutRow(editLayout, 5, "O", "Open new line above");
    addShortcutRow(editLayout, 6, "Esc", "Return to Normal mode");
    contentLayout->addWidget(editGroup);
    
    contentLayout->addStretch();
    
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
    
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    mainLayout->addWidget(buttons);
    
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &VimHelpDialog::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged, this, &VimHelpDialog::applyTranslations);
    
    applyTranslations();
    applyTheme();
}

void VimHelpDialog::applyTheme() {
    auto &tm = ThemeManager::instance();
    
    setStyleSheetIfChanged(this, QString("QDialog { background: %1; color: %2; }")
                                 .arg(tm.semanticColor(ThemeManager::SemanticRole::Canvas).name(),
                                      tm.semanticColor(ThemeManager::SemanticRole::Text).name()));
    const QString border = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString muted = tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
    const QString selected = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString onAccent = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();

    for (QWidget *child : findChildren<QWidget *>())
        child->setStyleSheet({});

    const QList<QLabel *> labels = findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->objectName() == QStringLiteral("vimKeyLabel")) {
            label->setStyleSheet(
                QString("font-family: monospace; font-weight: bold; font-size: 13px; "
                        "color: %1; padding: 2px 4px; background: %2; border-radius: 4px;")
                    .arg(onAccent, selected));
        } else if (label->text() == QStringLiteral("Vim Motions")) {
            label->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;")
                                     .arg(text));
        } else {
            label->setStyleSheet(QString("color: %1;").arg(muted));
        }
    }

    const QList<QGroupBox *> groups = findChildren<QGroupBox *>();
    for (QGroupBox *group : groups) {
        group->setStyleSheet(QString("QGroupBox { color: %1; border: 1px solid %2; "
                                     "border-radius: 6px; margin-top: 8px; padding: 8px; } "
                                     "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }")
                                 .arg(text, border));
    }

    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons)
        button->setStyleSheet(QString("color: %1;").arg(text));
}

void VimHelpDialog::applyTranslations() {
    setWindowTitle("Vim Motions Help");
}
