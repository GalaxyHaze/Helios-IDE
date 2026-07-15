#include "SearchPanel.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
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
}

SearchPanel::SearchPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    auto *title = new QLabel("Search");
    title->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px;");
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
    m_summaryLabel->setStyleSheet("color: #8c8fa1; font-size: 12px;");
    layout->addWidget(m_summaryLabel);

    m_results = new QListWidget;
    m_results->setWordWrap(true);
    m_results->setStyleSheet(
        "QListWidget { background: #11111b; color: #c6d0f5; border: 1px solid #363a4f; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #1e1e2e; }"
        "QListWidget::item:selected { background: #363a4f; }"
    );
    layout->addWidget(m_results, 1);

    setStyleSheet(
        "SearchPanel { background: #11111b; }"
        "QLineEdit { background: #1e1e2e; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; padding: 6px 8px; }"
        "QPushButton { background: #363a4f; color: #c6d0f5; border: none; border-radius: 4px; padding: 6px 10px; }"
        "QPushButton:hover { background: #45475a; }"
    );

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

    QDir root(m_rootPath);
    QDirIterator fileIterator(
        m_rootPath,
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);

    int totalResults = 0;
    while (fileIterator.hasNext() && totalResults < kMaxResults) {
        const QString path = fileIterator.next();
        if (!shouldScanFile(path)) {
            continue;
        }

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        int lineNumber = 0;
        int perFileResults = 0;
        while (!stream.atEnd() && totalResults < kMaxResults && perFileResults < kMaxResultsPerFile) {
            const QString line = stream.readLine();
            ++lineNumber;

            const qsizetype columnIndex = line.indexOf(needle, 0, Qt::CaseInsensitive);
            if (columnIndex < 0) {
                continue;
            }

            const QString relativePath = root.relativeFilePath(path);
            const QString preview = line.simplified();
            auto *item = new QListWidgetItem(
                QString("%1:%2  %3").arg(relativePath).arg(lineNumber).arg(preview),
                m_results);
            item->setToolTip(QDir::toNativeSeparators(path));
            item->setData(Qt::UserRole, path);
            item->setData(Qt::UserRole + 1, lineNumber - 1);
            item->setData(Qt::UserRole + 2, static_cast<int>(columnIndex));

            ++totalResults;
            ++perFileResults;
        }
    }

    if (totalResults == 0) {
        m_summaryLabel->setText(QString("No matches for \"%1\".").arg(needle));
    } else if (totalResults == kMaxResults) {
        m_summaryLabel->setText(QString("Showing the first %1 matches for \"%2\".").arg(kMaxResults).arg(needle));
    } else {
        m_summaryLabel->setText(QString("Found %1 matches for \"%2\".").arg(totalResults).arg(needle));
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
