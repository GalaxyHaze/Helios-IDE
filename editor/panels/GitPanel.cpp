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
#include <QInputDialog>

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

    // 1. Header Row (Title + Branch Pill)
    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);

    auto *title = new QLabel("Source Control");
    title->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px;");
    headerRow->addWidget(title);
    
    m_branchLabel = new QLabel("...");
    m_branchLabel->setStyleSheet("color: #8caaee; border: 1px solid #363a4f; border-radius: 4px; padding: 2px 6px; font-size: 11px; font-weight: bold; background: #181825;");
    headerRow->addWidget(m_branchLabel);
    headerRow->addStretch();

    auto *refreshButton = new QPushButton("Refresh");
    refreshButton->setStyleSheet("background: transparent; color: #8caaee; border: none; font-size: 11px;");
    refreshButton->setCursor(Qt::PointingHandCursor);
    headerRow->addWidget(refreshButton);
    layout->addLayout(headerRow);

    // Initializer buttons
    m_initButton = new QPushButton("Initialize Git Repository");
    m_initButton->setStyleSheet("background: #a6d189; color: #11111b; font-weight: bold; padding: 6px; border-radius: 4px;");
    layout->addWidget(m_initButton);
    
    m_connectGithubButton = new QPushButton("Connect to GitHub");
    m_connectGithubButton->setStyleSheet("background: #8caaee; color: #11111b; font-weight: bold; padding: 6px; border-radius: 4px;");
    layout->addWidget(m_connectGithubButton);
    m_initButton->hide();
    m_connectGithubButton->hide();

    // 2. Commit Box Section (VS Code Style)
    auto *commitGroup = new QWidget;
    auto *commitLayout = new QVBoxLayout(commitGroup);
    commitLayout->setContentsMargins(0, 4, 0, 4);
    commitLayout->setSpacing(6);
    
    m_commitInput = new QLineEdit;
    m_commitInput->setPlaceholderText("Message (Enter to commit)");
    m_commitInput->setStyleSheet("QLineEdit { background: #1e1e2e; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; padding: 6px 8px; font-size: 12px; }");
    commitLayout->addWidget(m_commitInput);
    
    m_commitButton = new QPushButton("Commit");
    m_commitButton->setStyleSheet("QPushButton { background: #7287fd; color: #11111b; font-weight: bold; border: none; border-radius: 4px; padding: 6px 10px; font-size: 12px; } QPushButton:hover { background: #8caaee; }");
    m_commitButton->setCursor(Qt::PointingHandCursor);
    commitLayout->addWidget(m_commitButton);
    layout->addWidget(commitGroup);

    // 3. Actions / Summary
    m_summaryLabel = new QLabel("Open a project inside a Git repository.");
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("color: #8c8fa1; font-size: 11px;");
    layout->addWidget(m_summaryLabel);

    auto *actionRow = new QHBoxLayout;
    actionRow->setContentsMargins(0, 0, 0, 0);
    actionRow->setSpacing(6);

    m_stageAllButton = new QPushButton("Stage all");
    m_stageSelectionButton = new QPushButton("Stage");
    m_unstageSelectionButton = new QPushButton("Unstage");
    
    QString actionStyle = "QPushButton { background: #363a4f; color: #c6d0f5; border: none; border-radius: 4px; padding: 4px 8px; font-size: 11px; } QPushButton:hover { background: #45475a; }";
    m_stageAllButton->setStyleSheet(actionStyle);
    m_stageSelectionButton->setStyleSheet(actionStyle);
    m_unstageSelectionButton->setStyleSheet(actionStyle);
    
    actionRow->addWidget(m_stageAllButton);
    actionRow->addWidget(m_stageSelectionButton);
    actionRow->addWidget(m_unstageSelectionButton);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    // 4. File List
    m_statusList = new QListWidget;
    m_statusList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_statusList->setStyleSheet(
        "QListWidget { background: #11111b; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; outline: none; }"
        "QListWidget::item { border-bottom: 1px solid transparent; }"
        "QListWidget::item:selected { background: #363a4f; }"
        "QListWidget::item:hover { background: #1e1e2e; }"
    );
    layout->addWidget(m_statusList, 1);

    setStyleSheet("GitPanel { background: #11111b; }");

    connect(refreshButton, &QPushButton::clicked, this, &GitPanel::refreshStatus);
    connect(m_initButton, &QPushButton::clicked, this, &GitPanel::initRepository);
    connect(m_connectGithubButton, &QPushButton::clicked, this, &GitPanel::connectToGithub);
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
    m_initButton->hide();
    m_connectGithubButton->hide();

    if (m_rootPath.isEmpty()) {
        m_branchLabel->setText("No Repo");
        setSummaryMessage("Open a project inside a Git repository.");
        return;
    }

    QString output;
    QString error;
    if (!runGit({"status", "--short", "--branch"}, &output, &error)) {
        m_branchLabel->setText("No Repo");
        setSummaryMessage(error.isEmpty()
            ? "Current workspace is not a Git repository."
            : error,
            true);
        m_initButton->show();
        return;
    }

    m_branchLabel->setText("...");
    
    // Check if origin exists
    QString remotes;
    if (runGit({"remote"}, &remotes, nullptr)) {
        if (!remotes.contains("origin")) {
            m_connectGithubButton->show();
        }
    }

    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        setSummaryMessage("Repository is clean.");
        return;
    }

    int changedFiles = 0;
    for (const QString &line : lines) {
        if (line.startsWith("##")) {
            const QString branchLine = line.mid(3).trimmed();
            m_branchLabel->setText(branchLine.isEmpty() ? "No Branch" : branchLine);
            continue;
        }

        QString statusStr = line.left(2);
        QString relativePath = line.mid(3).trimmed();
        const qsizetype renameArrow = relativePath.indexOf(" -> ");
        if (renameArrow >= 0)
            relativePath = relativePath.mid(renameArrow + 4).trimmed();

        auto *item = new QListWidgetItem();
        const QString absolutePath = QDir(m_rootPath).filePath(relativePath);
        if (QFileInfo::exists(absolutePath))
            item->setData(Qt::UserRole, absolutePath);
        item->setData(Qt::UserRole + 1, relativePath);
        item->setSizeHint(QSize(0, 26)); // Give some height for the custom widget
        m_statusList->addItem(item);
        
        // Build custom widget
        QWidget *rowWidget = new QWidget;
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(6, 2, 6, 2);
        rowLayout->setSpacing(8);
        
        QLabel *badge = new QLabel(statusStr.trimmed());
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(20, 16);
        
        // Colors based on status
        QString badgeBg = "#363a4f";
        QString badgeFg = "#c6d0f5";
        if (statusStr.contains("M")) {
            badgeBg = "#fab387"; // Peach
            badgeFg = "#11111b";
        } else if (statusStr.contains("A")) {
            badgeBg = "#a6d189"; // Green
            badgeFg = "#11111b";
        } else if (statusStr.contains("D")) {
            badgeBg = "#e82424"; // Red
            badgeFg = "#11111b";
        } else if (statusStr.contains("?")) {
            badgeBg = "#8caaee"; // Blue
            badgeFg = "#11111b";
        }
        
        badge->setStyleSheet(QString("background: %1; color: %2; border-radius: 4px; font-size: 10px; font-weight: bold; font-family: monospace;").arg(badgeBg, badgeFg));
        rowLayout->addWidget(badge);
        
        QLabel *nameLabel = new QLabel(QFileInfo(relativePath).fileName());
        nameLabel->setStyleSheet("color: #c6d0f5; font-size: 12px;");
        rowLayout->addWidget(nameLabel);
        
        QLabel *pathLabel = new QLabel(QFileInfo(relativePath).path());
        pathLabel->setStyleSheet("color: #6c7086; font-size: 11px;");
        rowLayout->addWidget(pathLabel, 1);
        
        m_statusList->setItemWidget(item, rowWidget);

        ++changedFiles;
    }

    if (changedFiles == 0) {
        setSummaryMessage("Repository is clean.");
    } else {
        setSummaryMessage(
            QString("%1 changed file(s). Select files to stage.")
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


void GitPanel::initRepository()
{
    if (m_rootPath.isEmpty()) return;
    
    QProcess git;
    git.setWorkingDirectory(m_rootPath);
    git.start("git", {"init", "-b", "main"});
    git.waitForFinished(kGitTimeoutMs);
    
    refreshStatus();
}

void GitPanel::connectToGithub()
{
    if (m_rootPath.isEmpty()) return;
    
    bool ok;
    QString url = QInputDialog::getText(this, "Connect to GitHub",
                                        "Enter GitHub Repository URL\(e.g., https://github.com/user/repo.git):",
                                        QLineEdit::Normal,
                                        "", &ok);
    if (ok && !url.isEmpty()) {
        QProcess git;
        git.setWorkingDirectory(m_rootPath);
        git.start("git", {"remote", "add", "origin", url.trimmed()});
        git.waitForFinished(kGitTimeoutMs);
        refreshStatus();
    }
}
