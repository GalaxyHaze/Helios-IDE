#include "SearchPanel.h"

#include "../core/AppearanceController.h"
#include "../core/ThemeManager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMetaObject>
#include <QThreadPool>
#include <QPushButton>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kRowSpacing = 6;
constexpr int kSearchDebounceMs = 250;
constexpr int kMaxResults = 200;
constexpr int kMaxResultsPerFile = 20;

class SearchRunnable : public QRunnable
{
public:
    SearchRunnable(QPointer<SearchPanel> panel,
                   QString rootPath,
                   QString needle,
                   qint64 token,
                   std::shared_ptr<std::atomic<qint64>> searchToken)
        : m_panel(panel)
        , m_rootPath(std::move(rootPath))
        , m_needle(std::move(needle))
        , m_token(token)
        , m_searchToken(std::move(searchToken))
    {
        setAutoDelete(true);
    }

    void run() override
    {
        if (!m_panel || !m_searchToken || m_searchToken->load() != m_token)
            return;

        QVector<SearchResult> results;
        int totalResults = 0;
        bool truncated = false;

        QDir root(m_rootPath);
        QDirIterator fileIterator(
            m_rootPath,
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);

        while (fileIterator.hasNext()
               && m_searchToken->load() == m_token
               && totalResults < kMaxResults) {
            const QString path = fileIterator.next();
            if (!SearchPanel::shouldScanFile(path))
                continue;

            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;

            QTextStream stream(&file);
            int lineNumber = 0;
            int perFileResults = 0;
            const QString relativePath = root.relativeFilePath(path);

            while (!stream.atEnd()
                   && m_searchToken->load() == m_token
                   && totalResults < kMaxResults
                   && perFileResults < kMaxResultsPerFile) {
                const QString line = stream.readLine();
                ++lineNumber;

                const qsizetype columnIndex = line.indexOf(m_needle, 0, Qt::CaseInsensitive);
                if (columnIndex < 0)
                    continue;

                SearchResult result;
                result.path = path;
                result.line = lineNumber;
                result.column = static_cast<int>(columnIndex);
                result.preview = QString("%1:%2  %3")
                                    .arg(relativePath)
                                    .arg(lineNumber)
                                    .arg(line.simplified());
                results.append(result);

                ++totalResults;
                ++perFileResults;

                if (results.size() >= 25)
                    flushResults();
            }
        }

        if (!m_searchToken || m_searchToken->load() != m_token)
            return;

        truncated = totalResults >= kMaxResults;
        flushResults();
        QMetaObject::invokeMethod(
            m_panel,
            [panel = m_panel, token = m_token, totalResults, truncated]() {
                if (panel)
                    panel->onSearchFinished(token, totalResults, truncated);
            },
            Qt::QueuedConnection);
    }

private:
    void flushResults()
    {
        if (m_results.isEmpty())
            return;

        const QVector<SearchResult> batch = m_results;
        m_results.clear();
        if (!m_panel || !m_searchToken || m_searchToken->load() != m_token)
            return;

        QMetaObject::invokeMethod(
            m_panel,
            [panel = m_panel, batch, token = m_token]() {
                if (panel)
                    panel->deliverSearchResults(batch, token);
            },
            Qt::QueuedConnection);
    }

    QPointer<SearchPanel> m_panel;
    QString m_rootPath;
    QString m_needle;
    qint64 m_token;
    std::shared_ptr<std::atomic<qint64>> m_searchToken;
    QVector<SearchResult> m_results;
};
}

SearchPanel::SearchPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    auto *title = new QLabel("Search");
    m_titleLabel = title;
    const auto& appearance = AppearanceController::instance();
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
    connect(m_searchTimer, &QTimer::timeout, this, &SearchPanel::triggerSearch);
    connect(searchButton, &QPushButton::clicked, this, &SearchPanel::triggerSearch);
    connect(m_results, &QListWidget::itemActivated, this, &SearchPanel::onItemActivated);
    connect(m_results, &QListWidget::itemClicked, this, &SearchPanel::onItemActivated);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SearchPanel::applyTheme);

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
        "QLineEdit { background: %5; color: %3; border: 1px solid %2; border-radius: 4px; padding: 6px 8px; }"
        "QPushButton { background: %6; color: %3; border: none; border-radius: 4px; padding: 6px 10px; }"
        "QPushButton:hover { background: %7; }")
        .arg(bg, border, text, muted, inputBg, buttonBg, buttonHover, selected, selectedText)
    );

    m_summaryLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: normal;").arg(muted));

    m_results->setStyleSheet(QString(
        "QListWidget { background: %1; color: %3; border: 1px solid %2; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid %2; }"
        "QListWidget::item:selected { background: %8; color: %9; }")
        .arg(bg, border, text, muted, inputBg, buttonBg, buttonHover, selected, selectedText)
    );

    m_titleLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: %2px;")
                                    .arg(text, QString::number(titleSize)));
}

void SearchPanel::setRootPath(const QString &path)
{
    m_rootPath = path;
    m_results->clear();

    if (m_rootPath.isEmpty()) {
        m_summaryLabel->setText("Open a project folder to enable workspace search.");
    } else {
        m_summaryLabel->setText(QString("Searching under %1").arg(QDir::toNativeSeparators(m_rootPath)));
    }
}

void SearchPanel::triggerSearch()
{
    const qint64 token = ++(*m_searchToken);
    m_results->clear();

    const QString needle = m_queryInput->text().trimmed();
    if (m_rootPath.isEmpty()) {
        m_summaryLabel->setText("Open a project folder to enable workspace search.");
        return;
    }
    if (needle.isEmpty()) {
        m_summaryLabel->setText(QString("Searching under %1").arg(QDir::toNativeSeparators(m_rootPath)));
        return;
    }

    auto runnable = new SearchRunnable(
        this,
        m_rootPath,
        needle,
        token,
        m_searchToken);
    QThreadPool::globalInstance()->start(runnable);
}

void SearchPanel::deliverSearchResults(const QVector<SearchResult> &results, qint64 token)
{
    if (token != m_searchToken->load())
        return;

    for (const SearchResult &result : results) {
        auto *item = new QListWidgetItem(result.preview, m_results);
        item->setToolTip(QDir::toNativeSeparators(result.path));
        item->setData(Qt::UserRole, result.path);
        item->setData(Qt::UserRole + 1, result.line - 1);
        item->setData(Qt::UserRole + 2, result.column);
    }
}

void SearchPanel::onSearchFinished(qint64 token, int totalResults, bool truncated)
{
    if (token != m_searchToken->load())
        return;

    const QString needle = m_queryInput->text().trimmed();
    if (totalResults == 0) {
        m_summaryLabel->setText(QString("No matches for \"%1\".").arg(needle));
    } else if (truncated) {
        m_summaryLabel->setText(QString("Showing the first %1 matches for \"%2\".")
                                    .arg(kMaxResults)
                                    .arg(needle));
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

bool SearchPanel::shouldScanFile(const QString &path)
{
    if (path.contains("/.git/") || path.contains("/build/") || path.contains("/venv/")) {
        return false;
    }

    const QString suffix = QFileInfo(path).suffix().toLower();
    static const QStringList textSuffixes = {
        "zith", "toml", "json", "md", "txt", "cpp", "cc", "cxx", "c", "h", "hpp", "qml"
    };
    return textSuffixes.contains(suffix);
}
