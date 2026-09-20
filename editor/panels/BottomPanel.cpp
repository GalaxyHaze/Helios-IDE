#include "BottomPanel.h"

#include "CompilerPanel.h"
#include "DiagnosticsPanel.h"
#include "ReferencesPanel.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet)
{
    if (widget->styleSheet() != styleSheet)
        widget->setStyleSheet(styleSheet);
}
}

BottomPanel::BottomPanel(QWidget *parent)
    : QDockWidget("Compiler Output", parent)
{
    setObjectName(QStringLiteral("bottomPanel"));
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    setMinimumHeight(140);

    auto *titleBar = new QWidget(this);
    titleBar->setFixedHeight(0);
    setTitleBarWidget(titleBar);

    auto *content = new QWidget(this);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *header = new QWidget(content);
    header->setObjectName(QStringLiteral("bottomPanelHeader"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 4, 8, 4);
    headerLayout->setSpacing(6);

    m_tabBar = new QTabBar(header);
    m_tabBar->setObjectName(QStringLiteral("bottomPanelTabBar"));
    m_tabBar->setDrawBase(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setUsesScrollButtons(false);
    m_tabBar->addTab(QStringLiteral("Diagnostics"));
    m_tabBar->addTab(QStringLiteral("Compiler Output"));
    m_tabBar->addTab(QStringLiteral("References"));
    headerLayout->addWidget(m_tabBar);
    headerLayout->addStretch(1);

    m_clearButton = new QToolButton(header);
    m_clearButton->setObjectName(QStringLiteral("bottomPanelClearButton"));
    m_clearButton->setText(QStringLiteral("Clear"));
    m_clearButton->setToolTip(QStringLiteral("Clear the current output tab"));
    headerLayout->addWidget(m_clearButton);

    m_closeButton = new QToolButton(header);
    m_closeButton->setObjectName(QStringLiteral("bottomPanelCloseButton"));
    m_closeButton->setText(QStringLiteral("Close"));
    m_closeButton->setToolTip(QStringLiteral("Close the bottom panel"));
    headerLayout->addWidget(m_closeButton);

    layout->addWidget(header);

    m_stack = new QStackedWidget(content);
    m_diagnostics = new DiagnosticsPanel(m_stack);
    m_compiler = new CompilerPanel(m_stack);
    m_references = new ReferencesPanel(m_stack);
    m_stack->addWidget(m_diagnostics);
    m_stack->addWidget(m_compiler);
    m_stack->addWidget(m_references);
    m_stack->setCurrentIndex(int(Tab::Compiler));
    layout->addWidget(m_stack, 1);

    setWidget(content);

    connect(m_tabBar, &QTabBar::currentChanged, m_stack,
            &QStackedWidget::setCurrentIndex);
    connect(m_clearButton, &QToolButton::clicked,
            this, &BottomPanel::clearCurrent);
    connect(m_closeButton, &QToolButton::clicked,
            this, &BottomPanel::onCloseClicked);
    connect(m_diagnostics, &DiagnosticsPanel::countsChanged,
            this, &BottomPanel::setDiagnosticsCount);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &BottomPanel::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &BottomPanel::applyTranslations);

    applyTranslations();
    applyTheme();
}

void BottomPanel::showTab(Tab tab)
{
    m_stack->setCurrentIndex(int(tab));
    m_tabBar->setCurrentIndex(int(tab));
    show();
}

void BottomPanel::clearCurrent()
{
    switch (Tab(m_tabBar->currentIndex())) {
    case Tab::Compiler:
        m_compiler->clearOutput();
        break;
    case Tab::References:
        m_references->clearReferences();
        break;
    case Tab::Diagnostics:
        break;
    }
}

void BottomPanel::setDiagnosticsCount(int errors, int warnings)
{
    m_errorCount = errors;
    m_warningCount = warnings;
    const int total = m_errorCount + m_warningCount;
    if (total == 0) {
        applyTranslations();
    } else {
        m_tabBar->setTabText(
            int(Tab::Diagnostics),
            QString("%1 (E%2 W%3)")
                .arg(TranslationManager::instance().translate(
                    "bottom.diagnostics"))
                .arg(m_errorCount)
                .arg(m_warningCount));
    }
}

void BottomPanel::onCloseClicked()
{
    hide();
    emit closeRequested();
}

void BottomPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QString canvas =
        tm.semanticColor(ThemeManager::SemanticRole::Canvas).name();
    const QString surface =
        tm.semanticColor(ThemeManager::SemanticRole::Surface).name();
    const QString surfaceAlt =
        tm.semanticColor(ThemeManager::SemanticRole::SurfaceAlt).name();
    const QString text =
        tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString muted =
        tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
    const QString border =
        tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString hover =
        tm.semanticColor(ThemeManager::SemanticRole::Hover).name();
    const QString selected =
        tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString selectedText =
        tm.semanticColor(ThemeManager::SemanticRole::SelectedText).name();

    setStyleSheetIfChanged(
        this,
        QString(
            "BottomPanel { background: %1; border-top: 1px solid %4; }"
            "#bottomPanelHeader { background: %2; border-bottom: 1px solid %4; }"
            "BottomPanel QTabBar::tab { background: transparent; color: %6; "
            "border: 1px solid transparent; border-bottom: none; "
            "border-top-left-radius: 4px; border-top-right-radius: 4px; "
            "padding: 5px 12px; }"
            "BottomPanel QTabBar::tab:selected { background: %1; color: %8; }"
            "BottomPanel QTabBar::tab:hover:!selected { background: %7; }"
            "BottomPanel QToolButton { background: %3; color: %5; border: none; "
            "border-radius: 4px; padding: 4px 10px; }"
            "BottomPanel QToolButton:hover { background: %7; }"
            "BottomPanel QToolButton:pressed { background: %9; }")
            .arg(canvas, surface, surfaceAlt, border, text, muted, hover,
                 selectedText, selected));
}

void BottomPanel::applyTranslations()
{
    const auto &tr = TranslationManager::instance();
    const QString diagnostics = tr.translate("bottom.diagnostics");
    const QString compiler = tr.translate("bottom.compiler_output");
    const QString references = tr.translate("bottom.references");
    m_tabBar->setTabText(int(Tab::Diagnostics), diagnostics);
    m_tabBar->setTabText(int(Tab::Compiler), compiler);
    m_tabBar->setTabText(int(Tab::References), references);
    m_clearButton->setText(tr.translate("bottom.clear"));
    m_clearButton->setToolTip(tr.translate("bottom.clear_tooltip"));
    m_closeButton->setText(tr.translate("bottom.close"));
    m_closeButton->setToolTip(tr.translate("bottom.close_tooltip"));

    if (m_errorCount > 0 || m_warningCount > 0) {
        m_tabBar->setTabText(
            int(Tab::Diagnostics),
            QString("%1 (E%2 W%3)")
                .arg(tr.translate("bottom.diagnostics"))
                .arg(m_errorCount)
                .arg(m_warningCount));
    }
}
