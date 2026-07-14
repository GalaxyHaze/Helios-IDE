#include "GitPanel.h"

#include <QAbstractItemView>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kGitTimeoutMs = 3000;
}

GitPanel::GitPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);

    auto *title = new QLabel("Source Control");
    title->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px;");
    headerRow->addWidget(title);
    headerRow->addStretch();

    auto *refreshButton = new QPushButton("Refresh");
    headerRow->addWidget(refreshButton);
    layout->addLayout(headerRow);

    m_branchLabel = new QLabel("No repository detected");
    m_branchLabel->setWordWrap(true);
    m_branchLabel->setStyleSheet("color: #8caaee; font-size: 12px; font-weight: bold;");
    layout->addWidget(m_branchLabel);

    m_summaryLabel = new QLabel("Open a project inside a Git repository.");
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("color: #8c8fa1; font-size: 12px;");
    layout->addWidget(m_summaryLabel);

    auto *actionRow = new QHBoxLayout;
    actionRow->setContentsMargins(0, 0, 0, 0);
    actionRow->setSpacing(6);

    m_stageAllButton = new QPushButton("Stage all");
    m_stageSelectionButton = new QPushButton("Stage");
    m_unstageSelectionButton = new QPushButton("Unstage");
    actionRow->addWidget(m_stageAllButton);
    actionRow->addWidget(m_stageSelectionButton);
    actionRow->addWidget(m_unstageSelectionButton);
    layout->addLayout(actionRow);

    m_statusList = new QListWidget;
    m_statusList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_statusList->setStyleSheet(
        "QListWidget { background: #11111b; color: #c6d0f5; border: 1px solid #363a4f; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #1e1e2e; }"
        "QListWidget::item:selected { background: #363a4f; }"
    );
    layout->addWidget(m_statusList, 1);

    m_commitInput = new QLineEdit;
    m_commitInput->setPlaceholderText("Commit message");
    layout->addWidget(m_commitInput);

    m_commitButton = new QPushButton("Commit");
    layout->addWidget(m_commitButton);

    setStyleSheet(
        "GitPanel { background: #11111b; }"
        "QLineEdit { background: #1e1e2e; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; padding: 6px 8px; }"
        "QPushButton { background: #363a4f; color: #c6d0f5; border: none; border-radius: 4px; padding: 6px 10px; }"
        "QPushButton:hover { background: #45475a; }"
    );

    connect(refreshButton, &QPushButton::clicked, this, &GitPanel::refreshStatus);
    connect(m_stageAllButton, &QPushButton::clicked, this, &GitPanel::stageAll);
    connect(m_stageSelectionButton, &QPushButton::clicked, this, &GitPanel::stageSelected);
    connect(m_unstageSelectionButton, &QPushButton::clicked, this, &GitPanel::unstageSelected);
    connect(m_commitButton, &QPushButton::clicked, this, &GitPanel::commitChanges);
    connect(m_commitInput, &QLineEdit::returnPressed, this, &GitPanel::commitChanges);
    connect(m_statusList, &QListWidget::itemActivated, this, &GitPanel::onItemActivated);
    connect(m_statusList, &QListWidget::itemClicked, this, &GitPanel::onItemActivated);
}

void GitPanel::setRootPath(const QString &path)
{
    m_rootPath = path;
    refreshStatus();
}

void GitPanel::refreshStatus()
{
    m_statusList->clear();

    if (m_rootPath.isEmpty()) {
        m_branchLabel->setText("No repository detected");
        setSummaryMessage("Open a project inside a Git repository.");
        return;
    }

    QString output;
    QString error;
    if (!runGit({"status", "--short", "--branch"}, &output, &error)) {
        m_branchLabel->setText("No repository detected");
        setSummaryMessage(error.isEmpty()
            ? "Current workspace is not a Git repository."
            : error,
            true);
        return;
    }

    m_branchLabel->setText("Repository ready");

    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        setSummaryMessage("Repository is clean.");
        return;
    }

    int changedFiles = 0;
    for (const QString &line : lines) {
        if (line.startsWith("##")) {
            const QString branchLine = line.mid(3).trimmed();
            m_branchLabel->setText(branchLine.isEmpty() ? "Repository ready" : branchLine);

            auto *item = new QListWidgetItem(branchLine);
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setForeground(QColor("#8caaee"));
            m_statusList->addItem(item);
            continue;
        }

        QString relativePath = line.mid(3).trimmed();
        const qsizetype renameArrow = relativePath.indexOf(" -> ");
        if (renameArrow >= 0)
            relativePath = relativePath.mid(renameArrow + 4).trimmed();

        auto *item = new QListWidgetItem(line);
        const QString absolutePath = QDir(m_rootPath).filePath(relativePath);
        if (QFileInfo::exists(absolutePath))
            item->setData(Qt::UserRole, absolutePath);
        item->setData(Qt::UserRole + 1, relativePath);
        m_statusList->addItem(item);
        ++changedFiles;
    }

    if (changedFiles == 0) {
        setSummaryMessage("Repository is clean.");
    } else {
        setSummaryMessage(
            QString("%1 changed file(s). Click an entry to open it or select files to stage.")
                .arg(changedFiles));
    }
}

void GitPanel::stageAll()
{
    QString stdErr;
    if (!runGit({"add", "--all"}, nullptr, &stdErr)) {
        setSummaryMessage(stdErr.isEmpty() ? "Failed to stage all changes." : stdErr, true);
        return;
    }

    setSummaryMessage("Staged all changes.");
    refreshStatus();
}

void GitPanel::stageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to stage.", true);
        return;
    }

    QStringList args = {"add", "--"};
    args.append(relativePaths);

    QString stdErr;
    if (!runGit(args, nullptr, &stdErr)) {
        setSummaryMessage(stdErr.isEmpty() ? "Failed to stage selected files." : stdErr, true);
        return;
    }

    setSummaryMessage(QString("Staged %1 file(s).").arg(relativePaths.size()));
    refreshStatus();
}

void GitPanel::unstageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to unstage.", true);
        return;
    }

    QStringList args = {"restore", "--staged", "--"};
    args.append(relativePaths);

    QString stdErr;
    if (!runGit(args, nullptr, &stdErr)) {
        setSummaryMessage(stdErr.isEmpty() ? "Failed to unstage selected files." : stdErr, true);
        return;
    }

    setSummaryMessage(QString("Unstaged %1 file(s).").arg(relativePaths.size()));
    refreshStatus();
}

void GitPanel::commitChanges()
{
    const QString message = m_commitInput->text().trimmed();
    if (message.isEmpty()) {
        setSummaryMessage("Enter a commit message before committing.", true);
        return;
    }

    QString stdOut;
    QString stdErr;
    if (!runGit({"commit", "-m", message}, &stdOut, &stdErr)) {
        setSummaryMessage(stdErr.isEmpty() ? "Commit failed." : stdErr, true);
        return;
    }

    m_commitInput->clear();
    setSummaryMessage(stdOut.simplified().isEmpty()
        ? "Commit created successfully."
        : stdOut.simplified());
    refreshStatus();
}

void GitPanel::onItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty())
        emit fileActivated(path);
}

bool GitPanel::runGit(const QStringList &args, QString *stdOut, QString *stdErr)
{
    if (m_rootPath.isEmpty()) {
        if (stdErr)
            *stdErr = "Open a project inside a Git repository.";
        return false;
    }

    QProcess git;
    git.setWorkingDirectory(m_rootPath);
    git.start("git", args);

    if (!git.waitForFinished(kGitTimeoutMs)) {
        if (stdErr)
            *stdErr = "Timed out while running Git.";
        return false;
    }

    if (stdOut)
        *stdOut = QString::fromUtf8(git.readAllStandardOutput()).trimmed();
    if (stdErr)
        *stdErr = QString::fromUtf8(git.readAllStandardError()).trimmed();

    return git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0;
}

QStringList GitPanel::selectedRelativePaths() const
{
    QStringList relativePaths;
    const QList<QListWidgetItem *> items = m_statusList->selectedItems();
    for (QListWidgetItem *item : items) {
        const QString relativePath = item->data(Qt::UserRole + 1).toString();
        if (!relativePath.isEmpty())
            relativePaths << relativePath;
    }
    relativePaths.removeDuplicates();
    return relativePaths;
}

void GitPanel::setSummaryMessage(const QString &message, bool isError)
{
    m_summaryLabel->setText(message);
    m_summaryLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
        .arg(isError ? "#e78284" : "#8c8fa1"));
}
