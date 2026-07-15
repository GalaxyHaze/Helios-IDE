#include "WelcomeWidget.h"
#include "../core/TomlSettingsStore.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QStyle>
#include <QDebug>

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);

    // Left column: Recents
    auto *leftCol = new QVBoxLayout;
    leftCol->setSpacing(10);

    m_recentLabel = new QLabel;
    m_recentLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    leftCol->addWidget(m_recentLabel);

    m_recentList = new QListWidget;
    m_recentList->setStyleSheet(
        "QListWidget { border: 1px solid #363a4f; border-radius: 6px; padding: 5px; font-size: 13px; }"
        "QListWidget::item { padding: 8px 10px; border-radius: 4px; }"
        "QListWidget::item:hover { background: #363a4f; }"
        "QListWidget::item:selected { background: #363a4f; color: #ffffff; }"
    );
    leftCol->addWidget(m_recentList, 1);
    mainLayout->addLayout(leftCol, 3);

    // Right column: Welcome header, Buttons, Shortcuts, Tips
    auto *rightCol = new QVBoxLayout;
    rightCol->setSpacing(15);

    m_titleLabel = new QLabel("Helios");
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold;");
    rightCol->addWidget(m_titleLabel);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(10);

    m_openBtn = new QPushButton;
    m_openBtn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(m_openBtn);

    m_newBtn = new QPushButton;
    m_newBtn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(m_newBtn);
    rightCol->addLayout(btnLayout);

    // Shortcuts Section
    m_shortcutsTitle = new QLabel;
    m_shortcutsTitle->setStyleSheet("font-size: 14px; font-weight: bold; margin-top: 10px;");
    rightCol->addWidget(m_shortcutsTitle);

    m_shortcutTexts = new QLabel;
    m_shortcutTexts->setStyleSheet("font-size: 12px; color: #a5adce; line-height: 18px;");
    rightCol->addWidget(m_shortcutTexts);

    // Tips Section
    m_tipsTitle = new QLabel;
    m_tipsTitle->setStyleSheet("font-size: 14px; font-weight: bold; margin-top: 10px;");
    rightCol->addWidget(m_tipsTitle);

    m_tipText = new QLabel;
    m_tipText->setStyleSheet("font-size: 12px; color: #a5adce; line-height: 18px;");
    m_tipText->setWordWrap(true);
    rightCol->addWidget(m_tipText);

    rightCol->addStretch(1);
    mainLayout->addLayout(rightCol, 2);

    // Connect Actions
    connect(m_openBtn, &QPushButton::clicked, this, &WelcomeWidget::openFolderRequested);
    connect(m_newBtn, &QPushButton::clicked, this, &WelcomeWidget::newProjectRequested);

    connect(m_recentList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        emit projectSelected(item->text());
    });

    // Listen to changes
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &WelcomeWidget::updateThemeAndLanguage);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged, this, &WelcomeWidget::updateThemeAndLanguage);

    updateThemeAndLanguage();
    refreshRecentProjects();
}

void WelcomeWidget::refreshRecentProjects()
{
    m_recentList->clear();
    QStringList recents = TomlSettingsStore::instance().recentProjects();
    for (const QString &path : recents) {
        m_recentList->addItem(path);
    }
}

void WelcomeWidget::updateThemeAndLanguage()
{
    auto &tm = ThemeManager::instance();
    auto &tr = TranslationManager::instance();

    m_recentLabel->setText(tr.translate("welcome.recent"));
    m_openBtn->setText(tr.translate("welcome.open"));
    m_newBtn->setText(tr.translate("welcome.new"));
    m_shortcutsTitle->setText(tr.translate("welcome.shortcuts"));
    m_tipsTitle->setText(tr.translate("welcome.tips"));

    m_shortcutTexts->setText(
        "Open Folder:\tCtrl+Alt+O\n"
        "New File:\tCtrl+N\n"
        "Save File:\tCtrl+S\n"
        "Explorer:\tCtrl+Shift+E\n"
        "Search Workspace:\tCtrl+Shift+F\n"
        "Build / Run:\tCtrl+B\n"
        "Getting Started:\tF1"
    );

    static int tipIdx = 0;
    tipIdx = (tipIdx + 1) % 2;
    if (tipIdx == 0) {
        m_tipText->setText(tr.translate("welcome.tip_explore"));
    } else {
        m_tipText->setText(tr.translate("welcome.tip_theme"));
    }

    QPalette pal = tm.palette();
    setPalette(pal);

    QColor base = pal.color(QPalette::Base);
    QColor text = pal.color(QPalette::Text);
    QColor windowText = pal.color(QPalette::WindowText);
    QColor highlight = pal.color(QPalette::Highlight);

    QString textHex = text.name();
    QString windowTextHex = windowText.name();
    QString bgHex = base.name();
    QString borderHex = tm.customColor("sidebarBorder", QColor("#363a4f")).name();
    QString itemHoverHex = tm.customColor("treeHover", QColor("#363a4f")).name();
    QString highlightHex = highlight.name();

    m_recentList->setStyleSheet(
        QString(
            "QListWidget { background: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 5px; font-size: 13px; }"
            "QListWidget::item { padding: 8px 10px; border-radius: 4px; color: %2; }"
            "QListWidget::item:hover { background: %4; }"
            "QListWidget::item:selected { background: %5; color: #ffffff; }"
        ).arg(bgHex, textHex, borderHex, itemHoverHex, highlightHex)
    );

    m_titleLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(windowTextHex));
    m_recentLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(windowTextHex));
    m_shortcutsTitle->setStyleSheet(QString("font-size: 14px; font-weight: bold; margin-top: 10px; color: %1;").arg(windowTextHex));
    m_tipsTitle->setStyleSheet(QString("font-size: 14px; font-weight: bold; margin-top: 10px; color: %1;").arg(windowTextHex));
    m_shortcutTexts->setStyleSheet(QString("font-size: 12px; color: %1; line-height: 18px;").arg(textHex));
    m_tipText->setStyleSheet(QString("font-size: 12px; color: %1; line-height: 18px;").arg(textHex));

    m_openBtn->setStyleSheet(
        QString(
            "QPushButton { background: %1; color: white; border: none; padding: 10px 15px; border-radius: 6px; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { opacity: 0.9; }"
        ).arg(highlightHex)
    );

    m_newBtn->setStyleSheet(
        QString(
            "QPushButton { background: %1; color: %2; border: 1px solid %3; padding: 10px 15px; border-radius: 6px; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { background: %4; }"
        ).arg(bgHex, textHex, borderHex, itemHoverHex)
    );
}
