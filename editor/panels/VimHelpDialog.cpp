#include "VimHelpDialog.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QScrollArea>

namespace {
void addShortcutRow(QGridLayout *layout, int row, const QString &keys, const QString &desc) {
    auto *keyLbl = new QLabel(keys);
    keyLbl->setStyleSheet("font-family: monospace; font-weight: bold; font-size: 13px; color: #7287fd; padding: 2px 4px; background: #363a4f; border-radius: 4px;");
    
    auto *descLbl = new QLabel(desc);
    
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
    title->setStyleSheet("font-size: 16px; font-weight: bold;");
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
    QPalette pal = tm.palette();
    
    setStyleSheetIfChanged(this, QString("QDialog { background: %1; color: %2; }")
                                 .arg(pal.color(QPalette::Window).name(), pal.color(QPalette::WindowText).name()));
}

void VimHelpDialog::applyTranslations() {
    setWindowTitle("Vim Motions Help");
}
