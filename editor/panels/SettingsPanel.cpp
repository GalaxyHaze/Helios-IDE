#include "SettingsPanel.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;

void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet)
{
    if (widget->styleSheet() != styleSheet)
        widget->setStyleSheet(styleSheet);
}
}

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    m_titleLabel = new QLabel("Settings");
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(m_titleLabel);

    m_preferencesCard = new QWidget(this);
    m_preferencesCard->setObjectName("preferencesCard");
    auto *preferencesLayout = new QVBoxLayout(m_preferencesCard);
    preferencesLayout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    preferencesLayout->setSpacing(6);

    m_preferencesTitleLabel = new QLabel("Appearance");
    m_preferencesTitleLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    preferencesLayout->addWidget(m_preferencesTitleLabel);

    m_hintLabel = new QLabel(
        "Theme, fonts, scale, and Vim are configured in Preferences.");
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setStyleSheet("font-size: 12px;");
    preferencesLayout->addWidget(m_hintLabel);

    m_openPreferencesButton = new QPushButton("Open Preferences...");
    m_openPreferencesButton->setObjectName("openPreferencesButton");
    preferencesLayout->addWidget(m_openPreferencesButton, 0, Qt::AlignLeft);

    m_openShortcutsButton = new QPushButton("Open Shortcuts Window...");
    m_openShortcutsButton->setObjectName("openShortcutsButton");
    preferencesLayout->addWidget(m_openShortcutsButton, 0, Qt::AlignLeft);

    m_openLspManagerButton = new QPushButton("Open LSP Manager...");
    m_openLspManagerButton->setObjectName("openLspManagerButton");
    preferencesLayout->addWidget(m_openLspManagerButton, 0, Qt::AlignLeft);
    layout->addWidget(m_preferencesCard);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    layout->addWidget(m_tabWidget, 1);

    // Appearance lives in the modal Preferences dialog.  The side panel is
    // intentionally reserved for shortcuts and runtime/LSP state.
    auto *shortcutsTab = new QWidget(this);
    auto *shortcutsLayout = new QVBoxLayout(shortcutsTab);
    shortcutsLayout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    shortcutsLayout->setSpacing(kPanelSpacing);

    m_shortcutsTree = new QTreeWidget(this);
    m_shortcutsTree->setColumnCount(2);
    m_shortcutsTree->setAlternatingRowColors(true);
    m_shortcutsTree->setSelectionMode(QAbstractItemView::NoSelection);
    m_shortcutsTree->setFocusPolicy(Qt::NoFocus);
    m_shortcutsTree->setAnimated(true);
    m_shortcutsTree->header()->setStretchLastSection(true);
    m_shortcutsTree->header()->setDefaultSectionSize(140);
    shortcutsLayout->addWidget(m_shortcutsTree);

    m_tabWidget->addTab(shortcutsTab, "Shortcuts");
    initializeShortcutTree();

    // ──────────────── TAB 2: LSP & RUNTIME ────────────────
    auto *lspTab = new QWidget(this);
    auto *lspLayout = new QVBoxLayout(lspTab);
    lspLayout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    lspLayout->setSpacing(kPanelSpacing);

    m_runtimeTitleLabel = new QLabel("Zith Runtime");
    m_runtimeTitleLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    lspLayout->addWidget(m_runtimeTitleLabel);

    m_runtimeHintLabel = new QLabel(
        "Helios manages the cached Zith LSP and stdlib runtime used by this window.");
    m_runtimeHintLabel->setWordWrap(true);
    m_runtimeHintLabel->setStyleSheet("font-size: 12px;");
    lspLayout->addWidget(m_runtimeHintLabel);

    m_lspEnabledCheck = new QCheckBox("Enable LSP");
    lspLayout->addWidget(m_lspEnabledCheck);

    auto *runtimeForm = new QFormLayout;
    runtimeForm->setContentsMargins(0, 4, 0, 0);
    runtimeForm->setSpacing(kPanelSpacing);

    auto makeValueLabel = []() {
        auto *label = new QLabel("Unavailable");
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setStyleSheet("font-size: 12px;");
        return label;
    };

    m_runtimeStatusValue = makeValueLabel();
    m_runtimeTagValue = makeValueLabel();
    m_runtimeLspPathValue = makeValueLabel();
    m_runtimeStdlibPathValue = makeValueLabel();
    m_runtimeCachePathValue = makeValueLabel();

    runtimeForm->addRow("Status", m_runtimeStatusValue);
    runtimeForm->addRow("Tag", m_runtimeTagValue);
    runtimeForm->addRow("LSP", m_runtimeLspPathValue);
    runtimeForm->addRow("Stdlib", m_runtimeStdlibPathValue);
    runtimeForm->addRow("Cache", m_runtimeCachePathValue);
    lspLayout->addLayout(runtimeForm);

    auto *runtimeButtons = new QHBoxLayout;
    runtimeButtons->setContentsMargins(0, 0, 0, 0);
    runtimeButtons->setSpacing(6);

    m_refreshRuntimeButton = new QPushButton("Refresh runtime");
    m_clearRuntimeCacheButton = new QPushButton("Clear cache");
    runtimeButtons->addWidget(m_refreshRuntimeButton);
    runtimeButtons->addWidget(m_clearRuntimeCacheButton);
    runtimeButtons->addStretch();
    lspLayout->addLayout(runtimeButtons);

    m_diagTitleLabel = new QLabel("LSP Diagnostics");
    m_diagTitleLabel->setStyleSheet("font-weight: bold; font-size: 13px; padding-top: 8px;");
    lspLayout->addWidget(m_diagTitleLabel);

    auto *diagForm = new QFormLayout;
    diagForm->setContentsMargins(0, 4, 0, 0);
    diagForm->setSpacing(kPanelSpacing);

    m_lspConnectionValue = makeValueLabel();
    m_lspSyncModeValue = makeValueLabel();
    m_lspLastErrorValue = makeValueLabel();
    m_lspLastErrorValue->setStyleSheet("color: #e78284; font-size: 12px;");

    diagForm->addRow("Connection", m_lspConnectionValue);
    diagForm->addRow("Sync mode", m_lspSyncModeValue);
    diagForm->addRow("Last error", m_lspLastErrorValue);
    lspLayout->addLayout(diagForm);

    m_logLabel = new QLabel("Recent LSP log");
    m_logLabel->setStyleSheet("font-size: 12px; padding-top: 4px;");
    lspLayout->addWidget(m_logLabel);

    m_lspLogView = new QPlainTextEdit;
    m_lspLogView->setReadOnly(true);
    m_lspLogView->setMaximumBlockCount(200);
    m_lspLogView->setFixedHeight(140);
    lspLayout->addWidget(m_lspLogView);

    lspLayout->addStretch();
    m_tabWidget->addTab(lspTab, "LSP");

    connect(m_openPreferencesButton, &QPushButton::clicked,
            this, &SettingsPanel::openPreferencesRequested);
    connect(m_openShortcutsButton, &QPushButton::clicked,
            this, &SettingsPanel::openShortcutsRequested);
    connect(m_openLspManagerButton, &QPushButton::clicked,
            this, &SettingsPanel::openLspManagerRequested);
    connect(m_lspEnabledCheck, &QCheckBox::toggled,
            this, &SettingsPanel::lspEnabledChanged);
    connect(m_refreshRuntimeButton, &QPushButton::clicked,
            this, &SettingsPanel::refreshRuntimeRequested);
    connect(m_clearRuntimeCacheButton, &QPushButton::clicked,
            this, &SettingsPanel::clearRuntimeCacheRequested);


    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SettingsPanel::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &SettingsPanel::applyTranslations);

    applyTheme();
    applyTranslations();
}

void SettingsPanel::setFontFamily(const QString &family)
{
    Q_UNUSED(family);
}

void SettingsPanel::setFontSize(int pointSize)
{
    Q_UNUSED(pointSize);
}

void SettingsPanel::setWordWrapEnabled(bool enabled)
{
    Q_UNUSED(enabled);
}

void SettingsPanel::setTheme(const QString &themeName)
{
    Q_UNUSED(themeName);
}

void SettingsPanel::setLocale(const QString &locale)
{
    Q_UNUSED(locale);
}

void SettingsPanel::setLspEnabled(bool enabled)
{
    QSignalBlocker blocker(m_lspEnabledCheck);
    m_lspEnabledCheck->setChecked(enabled);
    m_refreshRuntimeButton->setEnabled(enabled);
}

void SettingsPanel::setRuntimeInfo(const QString &status,
                                   const QString &tag,
                                   const QString &lspPath,
                                   const QString &stdlibPath,
                                   const QString &cachePath)
{
    m_runtimeStatusValue->setText(status.isEmpty() ? "Unavailable" : status);
    m_runtimeTagValue->setText(tag.isEmpty() ? "Unavailable" : tag);
    m_runtimeLspPathValue->setText(lspPath.isEmpty() ? "Unavailable" : lspPath);
    m_runtimeStdlibPathValue->setText(stdlibPath.isEmpty() ? "Unavailable" : stdlibPath);
    m_runtimeCachePathValue->setText(cachePath.isEmpty() ? "Unavailable" : cachePath);
}

void SettingsPanel::setLspDiagnostics(const QString &connection,
                                      const QString &syncMode,
                                      const QString &lastError)
{
    if (m_lspConnectionValue)
        m_lspConnectionValue->setText(connection.isEmpty() ? "Unknown" : connection);
    if (m_lspSyncModeValue)
        m_lspSyncModeValue->setText(syncMode.isEmpty() ? "Unknown" : syncMode);
    if (m_lspLastErrorValue)
        m_lspLastErrorValue->setText(lastError.isEmpty() ? "None" : lastError);
}

void SettingsPanel::appendLspLog(const QString &line)
{
    if (m_lspLogView && !line.isEmpty())
        m_lspLogView->appendPlainText(line);
}

void SettingsPanel::clearLspLog()
{
    if (m_lspLogView)
        m_lspLogView->clear();
}

void SettingsPanel::initializeShortcutTree()
{
    const auto addCategory = [this](const QString &categoryKey,
                                    const QList<QPair<QString, QString>> &items) {
        auto *categoryItem = new QTreeWidgetItem;
        categoryItem->setData(0, Qt::UserRole, categoryKey);
        categoryItem->setFont(0, QFont(QString(), -1, QFont::Bold));
        m_shortcutsTree->addTopLevelItem(categoryItem);

        for (const auto &item : items) {
            auto *child = new QTreeWidgetItem(categoryItem);
            child->setData(0, Qt::UserRole, item.first);
            child->setData(1, Qt::UserRole, item.second);
        }

        categoryItem->setExpanded(true);
    };

    addCategory("shortcut.cat.file", {
        { "shortcut.new_file", "Ctrl+N" },
        { "shortcut.new_window", "Ctrl+Shift+N" },
        { "shortcut.new_project", "Ctrl+Alt+N" },
        { "shortcut.open", "Ctrl+O" },
        { "shortcut.open_folder", "Ctrl+Alt+O" },
        { "shortcut.save", "Ctrl+S" },
        { "shortcut.exit", "Ctrl+Q" }
    });

    addCategory("shortcut.cat.edit_search", {
        { "shortcut.find", "Ctrl+F" },
        { "shortcut.replace", "Ctrl+H" },
        { "shortcut.find_next", "F3 / Ctrl+G" },
        { "shortcut.find_prev", "Shift+F3 / Ctrl+Shift+G" }
    });

    addCategory("shortcut.cat.navigation_view", {
        { "shortcut.explorer", "Ctrl+Shift+E" },
        { "shortcut.global_search", "Ctrl+Shift+F" },
        { "shortcut.git_panel", "Ctrl+Shift+G" },
        { "shortcut.settings", "Ctrl+," },
        { "shortcut.hide_sidebar", "Ctrl+Shift+X" },
        { "shortcut.go_back", "Alt+Left" },
        { "shortcut.go_forward", "Alt+Right" }
    });

    addCategory("shortcut.cat.lsp_tools", {
        { "shortcut.go_definition", "F12 / Ctrl+Click" },
        { "shortcut.go_implementation", "Ctrl+F12" },
        { "shortcut.format_doc", "Ctrl+Alt+L / Alt+Shift+F" },
        { "shortcut.completion", "Ctrl+Space" },
        { "shortcut.build", "Ctrl+B" },
        { "shortcut.check", "Ctrl+Shift+C" },
        { "shortcut.compile", "Ctrl+Shift+B" },
        { "shortcut.run", "Ctrl+Shift+R" },
        { "shortcut.restart_lsp", "Ctrl+Shift+L" },
        { "shortcut.getting_started", "F1" }
    });
}

void SettingsPanel::updateShortcutTexts()
{
    auto &tr = TranslationManager::instance();

    m_shortcutsTree->setHeaderLabels({tr.translate("shortcut.header_action"),
                                      tr.translate("shortcut.header_key")});
    for (int categoryIndex = 0;
         categoryIndex < m_shortcutsTree->topLevelItemCount();
         ++categoryIndex) {
        QTreeWidgetItem *categoryItem =
            m_shortcutsTree->topLevelItem(categoryIndex);
        categoryItem->setText(0,
                              tr.translate(categoryItem->data(0, Qt::UserRole)
                                               .toString()));

        for (int itemIndex = 0; itemIndex < categoryItem->childCount();
             ++itemIndex) {
            QTreeWidgetItem *item = categoryItem->child(itemIndex);
            item->setText(0,
                          tr.translate(item->data(0, Qt::UserRole).toString()));
            item->setText(1, item->data(1, Qt::UserRole).toString());
        }
    }
}

void SettingsPanel::applyTranslations()
{
    auto &tr = TranslationManager::instance();

    m_titleLabel->setText(tr.translate("settings.title"));
    m_preferencesTitleLabel->setText(tr.translate("settings.preferences_title"));
    m_hintLabel->setText(tr.translate("settings.preferences_hint"));
    m_openPreferencesButton->setText(tr.translate("settings.preferences_button"));
    m_openShortcutsButton->setText(tr.translate("settings.shortcuts_button"));
    m_openLspManagerButton->setText(tr.translate("settings.lsp_button"));
    m_tabWidget->setTabText(0, tr.translate("settings.tab_shortcuts"));
    m_tabWidget->setTabText(1, tr.translate("settings.tab_lsp"));
    updateShortcutTexts();
}

void SettingsPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();

    QPalette pal = tm.palette();
    if (palette() != pal)
        setPalette(pal);

    QColor base = pal.color(QPalette::Base);
    QColor text = pal.color(QPalette::Text);
    QColor windowText = pal.color(QPalette::WindowText);
    QColor altBase = pal.color(QPalette::AlternateBase);
    QColor highlight = pal.color(QPalette::Highlight);

    QString textHex = text.name();
    QString bgHex = base.name();
    QString windowTextHex = windowText.name();
    QString altBaseHex = altBase.name();
    QString borderHex = tm.customColor("sidebarBorder", QColor("#363a4f")).name();
    QString highlightHex = highlight.name();
    QString itemHoverHex = tm.customColor("treeHover", QColor("#363a4f")).name();

    setStyleSheetIfChanged(
        m_preferencesCard,
        QString("#preferencesCard { background: %1; border: 1px solid %2; border-radius: 6px; }")
            .arg(altBaseHex, borderHex));

    setStyleSheetIfChanged(
        m_preferencesTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px;").arg(windowTextHex));
    setStyleSheetIfChanged(
        m_titleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px;").arg(windowTextHex));
    if (m_hintLabel) {
        setStyleSheetIfChanged(m_hintLabel,
                               QString("color: %1; font-size: 12px;").arg(textHex));
    }
    setStyleSheetIfChanged(
        m_runtimeTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px;").arg(windowTextHex));
    setStyleSheetIfChanged(
        m_runtimeHintLabel,
        QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_diagTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px; padding-top: 8px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_logLabel,
        QString("color: %1; font-size: 12px; padding-top: 4px;").arg(textHex));
    setStyleSheetIfChanged(
        m_lspLastErrorValue,
        QString("color: %1; font-size: 12px;")
            .arg(pal.color(QPalette::BrightText).name()));

    setStyleSheetIfChanged(
        m_lspLogView,
        QString("QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-family: monospace; font-size: 11px; }")
            .arg(altBaseHex, textHex, borderHex));

    setStyleSheetIfChanged(
        m_tabWidget,
        QString(
            "QTabWidget::pane { border: 1px solid %1; border-radius: 4px; background: %2; }"
            "QTabBar::tab { background: %3; color: %4; padding: 4px 8px; border-top-left-radius: 4px; border-top-right-radius: 4px; font-size: 11px; }"
            "QTabBar::tab:selected { background: %2; color: %5; border: 1px solid %1; border-bottom: none; }"
            "QTabBar::tab:hover:!selected { background: %6; }"
        ).arg(borderHex, bgHex, altBaseHex, textHex, windowTextHex, itemHoverHex));

    setStyleSheetIfChanged(
        m_shortcutsTree,
        QString(
            "QTreeWidget { background: %1; color: %2; border: none; font-size: 12px; }"
            "QTreeWidget::item { padding: 4px; color: %2; }"
            "QHeaderView::section { background: %3; color: %2; border: 1px solid %1; font-weight: bold; font-size: 11px; padding: 4px; }"
        ).arg(altBaseHex, textHex, bgHex));

    setStyleSheetIfChanged(
        this,
        QString(
            "SettingsPanel { background: %1; color: %2; }"
            "QSpinBox { background: %3; color: %2; border: 1px solid %4; border-radius: 4px; padding: 4px 6px; }"
            "QComboBox { background: %3; color: %2; border: 1px solid %4; border-radius: 4px; padding: 4px 6px; }"
            "QComboBox QAbstractItemView { background: %3; color: %2; selection-background-color: %5; }"
            "QPushButton { background: %4; color: %2; border: none; border-radius: 4px; padding: 6px 10px; }"
            "QPushButton:hover { background: %6; }"
            "QCheckBox { color: %2; }"
            "QLabel { color: %2; }"
        ).arg(bgHex, textHex, altBaseHex, borderHex, highlightHex, itemHoverHex));
}
