#include "GettingStartedDialog.h"

#include "../core/ThemeManager.h"
#include "../core/TomlSettingsStore.h"
#include "../core/TranslationManager.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

GettingStartedDialog::GettingStartedDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Getting Started with Helios"));
    resize(500, 400);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *title = new QLabel(QStringLiteral("Welcome to Helios!"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: bold;"));
    layout->addWidget(title);

    auto *intro = new QLabel(
        QStringLiteral("Helios is a high-performance C++/Zith "
                       "development editor inspired by JetBrains CLion."),
        this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *shortcutsGroup = new QGroupBox(QStringLiteral("Core Shortcuts"), this);
    auto *shortcutsLayout = new QGridLayout(shortcutsGroup);
    shortcutsLayout->setSpacing(10);

    int row = 0;
    const auto addShortcut = [&](const QString &description,
                                 const QString &keys) {
        auto *descriptionLabel = new QLabel(description, this);
        auto *keysLabel = new QLabel(keys, this);
        keysLabel->setStyleSheet(
            QStringLiteral("font-weight: bold; color: %1;")
                .arg(ThemeManager::instance()
                         .semanticColor(ThemeManager::SemanticRole::Accent)
                         .name()));
        shortcutsLayout->addWidget(descriptionLabel, row, 0);
        shortcutsLayout->addWidget(keysLabel, row, 1);
        ++row;
    };

    addShortcut(QStringLiteral("Open Folder"), QStringLiteral("Ctrl+Alt+O"));
    addShortcut(QStringLiteral("New File"), QStringLiteral("Ctrl+N"));
    addShortcut(QStringLiteral("Toggle Project Explorer"),
                QStringLiteral("Ctrl+Shift+E"));
    addShortcut(QStringLiteral("Search Workspace"),
                QStringLiteral("Ctrl+Shift+F"));
    addShortcut(QStringLiteral("Build Project"), QStringLiteral("Ctrl+B"));
    addShortcut(QStringLiteral("Go to Definition"),
                QStringLiteral("Ctrl+Click or F12"));
    layout->addWidget(shortcutsGroup);

    auto *tipsGroup = new QGroupBox(QStringLiteral("Quick Tips"), this);
    auto *tipsLayout = new QVBoxLayout(tipsGroup);
    tipsLayout->setSpacing(5);
    tipsLayout->addWidget(new QLabel(
        QStringLiteral("• Double-click files in the Explorer to open them."),
        this));
    tipsLayout->addWidget(new QLabel(
        QStringLiteral("• ") +
            TranslationManager::instance().translate(QStringLiteral("welcome.tip_theme")),
        this));
    tipsLayout->addWidget(new QLabel(
        QStringLiteral("• Right-click in the editor or file explorer to "
                       "access rich context actions."),
        this));
    layout->addWidget(tipsGroup);

    auto *bottomLayout = new QHBoxLayout();
    auto *dontShowAgain =
        new QCheckBox(QStringLiteral("Do not show this on startup"), this);
    dontShowAgain->setChecked(
        TomlSettingsStore::instance().onboardingDismissed());
    bottomLayout->addWidget(dontShowAgain);
    bottomLayout->addStretch();

    auto *closeButton = new QPushButton(QStringLiteral("Close"), this);
    closeButton->setStyleSheet(
        QStringLiteral("background: %1; color: %2; padding: 6px 15px; "
                       "border: none; border-radius: 4px; font-weight: bold;")
            .arg(ThemeManager::instance()
                     .semanticColor(ThemeManager::SemanticRole::Accent)
                     .name(),
                 ThemeManager::instance()
                     .semanticColor(ThemeManager::SemanticRole::OnAccent)
                     .name()));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(closeButton);
    layout->addLayout(bottomLayout);

    auto &theme = ThemeManager::instance();
    setStyleSheet(
        QStringLiteral("QDialog { background: %1; color: %2; }"
                       "QGroupBox { font-weight: bold; border: 1px solid %3; "
                       "border-radius: 6px; margin-top: 10px; padding-top: 15px; "
                       "color: %2; }"
                       "QGroupBox::title { subcontrol-origin: margin; left: 10px; "
                       "padding: 0 3px; color: %2; }"
                       "QLabel { color: %2; }"
                       "QCheckBox { color: %2; }")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::Canvas).name(),
                 theme.semanticColor(ThemeManager::SemanticRole::Text).name(),
                 theme.semanticColor(ThemeManager::SemanticRole::Border).name()));

    connect(dontShowAgain, &QCheckBox::checkStateChanged, this,
            [](int state) {
                TomlSettingsStore::instance().setOnboardingDismissed(
                    state == Qt::Checked);
            });
}
