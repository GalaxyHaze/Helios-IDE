#include "SearchPanel.h"

#include "../core/AppearanceController.h"
#include "../core/ThemeManager.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <utility>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kRowSpacing = 6;
constexpr int kSearchDebounceMs = 250;
}

SearchPanel::SearchPanel(
    WorkspaceSearchController::ScanPolicyProvider scanPolicyProvider,
    QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    auto *title = new QLabel("Search");
    m_titleLabel = title;
    layout->addWidget(title);

    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(kRowSpacing);

    m_queryInput = new QLineEdit;
    m_queryInput->setPlaceholderText("Search in project...");
    row->addWidget(m_queryInput, 1);

    auto *searchButton = new QPushButton("Find");
    row->addWidget(searchButton);

    layout->addLayout(row);

    auto *replaceRow = new QHBoxLayout;
    m_replaceInput = new QLineEdit;
    m_replaceInput->setPlaceholderText("Replace in project...");
    m_replaceInput->setClearButtonEnabled(true);
    replaceRow->addWidget(m_replaceInput, 1);

    m_replaceButton = new QPushButton("Replace All");
    replaceRow->addWidget(m_replaceButton);
    layout->addLayout(replaceRow);

    m_summaryLabel = new QLabel("Enter a query to search the current workspace.");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    m_results = new QListWidget;
    m_results->setWordWrap(true);
    layout->addWidget(m_results, 1);

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(kSearchDebounceMs);

    connect(m_queryInput, &QLineEdit::textChanged, this, [this]() {
        m_searchTimer->start();
    });
    connect(m_queryInput, &QLineEdit::returnPressed, this, &SearchPanel::triggerSearch);
    connect(m_replaceInput, &QLineEdit::returnPressed, this,
            &SearchPanel::triggerReplaceAll);
    connect(m_searchTimer, &QTimer::timeout, this, &SearchPanel::triggerSearch);
    connect(searchButton, &QPushButton::clicked, this, &SearchPanel::triggerSearch);
    connect(m_replaceButton, &QPushButton::clicked, this,
            &SearchPanel::triggerReplaceAll);
    connect(m_results, &QListWidget::itemActivated, this, &SearchPanel::onItemActivated);
    connect(m_results, &QListWidget::itemClicked, this, &SearchPanel::onItemActivated);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SearchPanel::applyTheme);

    m_searchController =
        new WorkspaceSearchController(std::move(scanPolicyProvider), this);
    connect(m_searchController, &WorkspaceSearchController::resultsReady,
            this, &SearchPanel::deliverSearchResults);
    connect(m_searchController,
            &WorkspaceSearchController::searchFinished, this,
            &SearchPanel::onSearchFinished);
    connect(m_searchController,
            &WorkspaceSearchController::replaceAllPreviewReady, this,
            &SearchPanel::replaceAllPreviewReady);

    applyTheme();
}

void SearchPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QString bg = tm.semanticColor(ThemeManager::SemanticRole::Surface).name();
    const QString border = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString muted = tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
    const QString inputBg = tm.semanticColor(ThemeManager::SemanticRole::InputBg).name();
    const QString buttonBg = tm.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
    const QString buttonHover = tm.semanticColor(ThemeManager::SemanticRole::ButtonHover).name();
    const QString selected = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString selectedText = tm.semanticColor(ThemeManager::SemanticRole::SelectedText).name();

    const int titleSize = qMax(
        AppearanceController::instance().uiFont().pointSize() - 2,
        AppearanceController::instance().minFontSize());

    setStyleSheet(QString(
        "SearchPanel { background: %1; }"
        "QLineEdit { background: %2; color: %3; border: 1px solid %4; border-radius: 6px; padding: 6px 8px; }"
        "QPushButton { background: %5; color: %3; border: none; border-radius: 6px; padding: 6px 10px; }"
        "QPushButton:hover { background: %6; }")
        .arg(bg, inputBg, text, border, buttonBg, buttonHover)
    );

    m_summaryLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: normal;").arg(muted));

    m_results->setStyleSheet(QString(
        "QListWidget { background: %1; color: %2; border: 1px solid %3; "
        "border-radius: 6px; }"
        "QListWidget::item { padding: 7px 10px; border-radius: 6px; }"
        "QListWidget::item:selected { background: %4; color: %5; }")
        .arg(bg, text, border, selected, selectedText)
    );

    m_titleLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: %2px;")
                                    .arg(text, QString::number(titleSize)));
}

void SearchPanel::setRootPath(const QString &path)
{
    m_searchController->setRootPath(path);
    m_results->clear();

    if (path.isEmpty()) {
        m_summaryLabel->setText("Open a project folder to enable workspace search.");
    } else {
        m_summaryLabel->setText(QString("Searching under %1").arg(QDir::toNativeSeparators(path)));
    }
}

QString SearchPanel::rootPath() const
{
    return m_searchController ? m_searchController->rootPath() : QString();
}

void SearchPanel::triggerSearch()
{
    m_results->clear();

    const QString needle = m_queryInput->text().trimmed();
    if (rootPath().isEmpty()) {
        m_summaryLabel->setText("Open a project folder to enable workspace search.");
        return;
    }
    if (needle.isEmpty()) {
        m_summaryLabel->setText(QString("Searching under %1").arg(
            QDir::toNativeSeparators(rootPath())));
        return;
    }

    m_searchController->search(needle);
}

void SearchPanel::triggerReplaceAll()
{
    const QString needle = m_queryInput->text().trimmed();
    const QString replacement = m_replaceInput->text();
    if (rootPath().isEmpty() || needle.isEmpty())
        return;

    m_searchController->previewReplace(needle, replacement);
}

void SearchPanel::deliverSearchResults(const QVector<SearchResult> &results)
{
    for (const SearchResult &result : results) {
        auto *item = new QListWidgetItem(result.preview, m_results);
        item->setToolTip(QDir::toNativeSeparators(result.path));
        item->setData(Qt::UserRole, result.path);
        item->setData(Qt::UserRole + 1, result.line - 1);
        item->setData(Qt::UserRole + 2, result.column);
    }
}

void SearchPanel::onSearchFinished(int totalResults, bool truncated)
{
    const QString needle = m_queryInput->text().trimmed();
    if (totalResults == 0) {
        m_summaryLabel->setText(QString("No matches for \"%1\".").arg(needle));
    } else if (truncated) {
        m_summaryLabel->setText(
            QString("Showing a limited result set for \"%1\".").arg(needle));
    } else {
        m_summaryLabel->setText(QString("Found %1 matches for \"%2\".")
                                    .arg(totalResults)
                                    .arg(needle));
    }
}

void SearchPanel::onItemActivated(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    emit fileActivated(
        item->data(Qt::UserRole).toString(),
        item->data(Qt::UserRole + 1).toInt(),
        item->data(Qt::UserRole + 2).toInt());
}
