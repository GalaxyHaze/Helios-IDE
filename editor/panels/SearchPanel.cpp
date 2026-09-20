#include "SearchPanel.h"

#include "../core/AppearanceController.h"
#include "../core/ThemeManager.h"
#include "../core/TomlSettingsStore.h"

#include <QByteArray>
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
#include <QRegularExpression>
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
            const auto &settings = TomlSettingsStore::instance();
            if (!SearchPanel::shouldScanFile(path,
                                             settings.searchTextExtensions(),
                                             settings.searchExcludedDirs()))
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

class ReplaceScanRunnable : public QRunnable
{
public:
    ReplaceScanRunnable(QPointer<SearchPanel> panel,
                        QString rootPath,
                        QString needle,
                        QString replacement,
                        QStringList textExtensions,
                        QStringList excludedDirs,
                        qint64 token,
                        std::shared_ptr<std::atomic<qint64>> searchToken)
        : m_panel(panel)
        , m_rootPath(std::move(rootPath))
        , m_needle(std::move(needle))
        , m_replacement(std::move(replacement))
        , m_textExtensions(std::move(textExtensions))
        , m_excludedDirs(std::move(excludedDirs))
        , m_token(token)
        , m_searchToken(std::move(searchToken))
    {
        setAutoDelete(true);
    }

    void run() override
    {
        if (!m_panel || !m_searchToken || m_searchToken->load() != m_token)
            return;

        QVector<SearchReplaceTarget> targets;
        QDir root(m_rootPath);
        QDirIterator fileIterator(
            m_rootPath,
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);

        while (fileIterator.hasNext()
               && m_searchToken->load() == m_token) {
            const QString path = fileIterator.next();
            if (!SearchPanel::shouldScanFile(path, m_textExtensions, m_excludedDirs))
                continue;

            QFile file(path);
            if (!file.open(QIODevice::ReadOnly))
                continue;
            const QString text = QString::fromUtf8(file.readAll());
            file.close();

            const QList<QPair<LspRange, QString>> edits =
                SearchPanel::replaceEdits(text, m_needle, m_replacement);
            if (!edits.isEmpty()) {
                SearchReplaceTarget target;
                target.path = path;
                target.matches = edits.size();
                targets.append(target);
            }
        }

        if (!m_searchToken || m_searchToken->load() != m_token)
            return;

        QMetaObject::invokeMethod(
            m_panel,
            [panel = m_panel,
             needle = m_needle,
             replacement = m_replacement,
             targets]() {
                if (panel)
                    emit panel->replaceAllPreviewReady(needle, replacement, targets);
            },
            Qt::QueuedConnection);
    }

private:
    QPointer<SearchPanel> m_panel;
    QString m_rootPath;
    QString m_needle;
    QString m_replacement;
    QStringList m_textExtensions;
    QStringList m_excludedDirs;
    qint64 m_token;
    std::shared_ptr<std::atomic<qint64>> m_searchToken;
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

void SearchPanel::triggerReplaceAll()
{
    const QString needle = m_queryInput->text().trimmed();
    const QString replacement = m_replaceInput->text().trimmed();
    if (m_rootPath.isEmpty() || needle.isEmpty())
        return;

    const auto &settings = TomlSettingsStore::instance();
    auto runnable = new ReplaceScanRunnable(
        this,
        m_rootPath,
        needle,
        replacement,
        settings.searchTextExtensions(),
        settings.searchExcludedDirs(),
        ++(*m_searchToken),
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

bool SearchPanel::shouldScanFile(const QString &path,
                                 const QStringList &textExtensions,
                                 const QStringList &excludedDirs)
{
    const QString normalized = QDir::fromNativeSeparators(path);
    for (const QString &dir : excludedDirs) {
        if (dir.isEmpty())
            continue;
        if (normalized.contains(QStringLiteral("/%1/").arg(dir)) ||
            normalized.endsWith(QStringLiteral("/%1").arg(dir))) {
            return false;
        }
    }

    const QString fileName = QFileInfo(normalized).fileName().toLower();
    if (fileName.isEmpty())
        return false;

    const QString suffix = QFileInfo(normalized).suffix().toLower();
    if (!suffix.isEmpty() && textExtensions.contains(suffix))
        return true;

    return textExtensions.contains(fileName);
}

bool SearchPanel::shouldScanFile(const QString &path)
{
    const auto &settings = TomlSettingsStore::instance();
    return shouldScanFile(path,
                          settings.searchTextExtensions(),
                          settings.searchExcludedDirs());
}

QList<QPair<LspRange, QString>> SearchPanel::replaceEdits(
    const QString &text,
    const QString &needle,
    const QString &replacement)
{
    QList<QPair<LspRange, QString>> edits;
    if (needle.isEmpty())
        return edits;

    // The SearchPanel contract is case-insensitive because the existing
    // workspace finder is case-insensitive.  Keep replacement consistent
    // with that behavior.
    int offset = 0;
    while (true) {
        const int found = text.indexOf(needle, offset, Qt::CaseInsensitive);
        if (found < 0)
            break;

        LspRange range;
        const QString prefix = text.left(found);
        range.start.line = prefix.count('\n');
        const int lineStart = prefix.lastIndexOf('\n');
        range.start.character = lineStart < 0 ? found : found - lineStart - 1;
        range.end.line = range.start.line;
        range.end.character = range.start.character + needle.size();

        edits.append({range, replacement});
        offset = found + needle.size();
    }
    return edits;
}

QString SearchPanel::applyReplaceEdits(
    const QString &text,
    const QList<QPair<LspRange, QString>> &edits)
{
    QString result = text;
    for (auto it = edits.rbegin(); it != edits.rend(); ++it) {
        const LspRange &range = it->first;
        const int start = SearchPanel::offsetForPosition(result, range.start);
        const int end = SearchPanel::offsetForPosition(result, range.end);
        if (start < 0 || end < start)
            continue;
        result.replace(start, end - start, it->second);
    }
    return result;
}

int SearchPanel::offsetForPosition(const QString &text, const LspPosition &pos)
{
    int line = pos.line;
    int offset = 0;
    while (line > 0 && offset < text.size()) {
        const int next = text.indexOf('\n', offset);
        if (next < 0)
            return -1;
        offset = next + 1;
        --line;
    }
    if (offset + pos.character > text.size())
        return -1;
    return offset + pos.character;
}
