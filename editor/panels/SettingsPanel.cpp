#include "SettingsPanel.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QFontComboBox>
#include <QApplication>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kFontSizeMin = 8;
constexpr int kFontSizeMax = 32;

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

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    layout->addWidget(m_tabWidget, 1);

    // ──────────────── TAB 1: EDITOR ────────────────
    auto *editorTab = new QWidget(this);
    auto *editorLayout = new QVBoxLayout(editorTab);
    editorLayout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    editorLayout->setSpacing(kPanelSpacing);

    m_hintLabel = new QLabel("These options apply to the open editors in this window.");
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setStyleSheet("font-size: 12px;");
    editorLayout->addWidget(m_hintLabel);

    auto *form = new QFormLayout;
    form->setContentsMargins(0, 4, 0, 0);
    form->setSpacing(kPanelSpacing);

    m_fontFamilyCombo = new QFontComboBox;
    form->addRow("Font Family", m_fontFamilyCombo);
    
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(kFontSizeMin, kFontSizeMax);
    form->addRow("Font size", m_fontSizeSpin);

    m_wordWrapCheck = new QCheckBox("Wrap long lines");
    form->addRow(QString(), m_wordWrapCheck);

    m_themeCombo = new QComboBox;
    m_themeCombo->addItem("Helios Dark", "helios-dark");
    m_themeCombo->addItem("Helios Light", "helios-light");
    m_themeCombo->addItem("ColorMind Midnight", "colormind-midnight");
    m_themeCombo->addItem("ColorMind Desert", "colormind-desert");
    m_themeCombo->addItem("Helios Violet", "helios-violet-dark");
    m_themeCombo->addItem("Helios Red", "helios-red-dark");
    m_themeCombo->addItem("Helios Gold", "helios-gold-dark");
    m_themeCombo->addItem("Helios Blue", "helios-blue-dark");
    m_themeCombo->addItem("Helios Forest", "helios-forest-dark");
    m_themeCombo->addItem("Helios Carbon", "helios-carbon-dark");
    form->addRow("Theme", m_themeCombo);

    m_localeCombo = new QComboBox;
    m_localeCombo->addItem("English (US)", "en-US");
    m_localeCombo->addItem("Português (BR)", "pt-BR");
    form->addRow("Language", m_localeCombo);

    editorLayout->addLayout(form);
    editorLayout->addStretch();
    m_tabWidget->addTab(editorTab, "Editor");

    // ──────────────── TAB 2: SHORTCUTS ────────────────
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

    // ──────────────── TAB 3: LSP & RUNTIME ────────────────
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

    connect(m_fontFamilyCombo, &QFontComboBox::currentFontChanged,
            this, [this](const QFont &font) { emit fontFamilyChanged(font.family()); });
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPanel::fontSizeChanged);
    connect(m_wordWrapCheck, &QCheckBox::toggled,
            this, &SettingsPanel::wordWrapChanged);
    connect(m_lspEnabledCheck, &QCheckBox::toggled,
            this, &SettingsPanel::lspEnabledChanged);
    connect(m_refreshRuntimeButton, &QPushButton::clicked,
            this, &SettingsPanel::refreshRuntimeRequested);
    connect(m_clearRuntimeCacheButton, &QPushButton::clicked,
            this, &SettingsPanel::clearRuntimeCacheRequested);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        emit themeChanged(m_themeCombo->itemData(idx).toString());
    });
    connect(m_localeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        emit localeChanged(m_localeCombo->itemData(idx).toString());
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SettingsPanel::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &SettingsPanel::applyTranslations);

    applyTheme();
    applyTranslations();
}

void SettingsPanel::setFontFamily(const QString &family)
{
    QSignalBlocker blocker(m_fontFamilyCombo);
    if (family == "System Default" || family.isEmpty()) {
        m_fontFamilyCombo->setCurrentFont(QApplication::font());
    } else {
        m_fontFamilyCombo->setCurrentFont(QFont(family));
    }
}

void SettingsPanel::setFontSize(int pointSize)
{
    QSignalBlocker blocker(m_fontSizeSpin);
    m_fontSizeSpin->setValue(pointSize);
}

void SettingsPanel::setWordWrapEnabled(bool enabled)
{
    QSignalBlocker blocker(m_wordWrapCheck);
    m_wordWrapCheck->setChecked(enabled);
}

void SettingsPanel::setTheme(const QString &themeName)
{
    int idx = m_themeCombo->findData(themeName);
    if (idx != -1) {
        QSignalBlocker blocker(m_themeCombo);
        m_themeCombo->setCurrentIndex(idx);
    }
}

void SettingsPanel::setLocale(const QString &locale)
{
    int idx = m_localeCombo->findData(locale);
    if (idx != -1) {
        QSignalBlocker blocker(m_localeCombo);
        m_localeCombo->setCurrentIndex(idx);
    }
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

    m_tabWidget->setTabText(0, tr.translate("settings.tab_editor"));
    m_tabWidget->setTabText(1, tr.translate("settings.tab_shortcuts"));
    m_tabWidget->setTabText(2, tr.translate("settings.tab_lsp"));
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
        m_titleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px;").arg(windowTextHex));
    setStyleSheetIfChanged(m_hintLabel,
                           QString("color: %1; font-size: 12px;").arg(textHex));
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
