#include "GitPanel.h"
#include "../core/ThemeManager.h"

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
#include <QTimer>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kGitTimeoutMs = 10000;
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
    headerRow->addWidget(title);
    
    m_branchLabel = new QLabel("...");
    headerRow->addWidget(m_branchLabel);
    headerRow->addStretch();

    m_refreshButton = new QPushButton("Refresh");
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    headerRow->addWidget(m_refreshButton);
    layout->addLayout(headerRow);

    // Initializer buttons
    m_initButton = new QPushButton("Initialize Git Repository");
    layout->addWidget(m_initButton);
    
    m_connectGithubButton = new QPushButton("Connect to GitHub");
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
    commitLayout->addWidget(m_commitInput);
    
    m_commitButton = new QPushButton("Commit");
    m_commitButton->setCursor(Qt::PointingHandCursor);
    commitLayout->addWidget(m_commitButton);
    layout->addWidget(commitGroup);

    // 3. Actions / Summary
    m_summaryLabel = new QLabel("Open a project inside a Git repository.");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    auto *actionRow = new QHBoxLayout;
    actionRow->setContentsMargins(0, 0, 0, 0);
    actionRow->setSpacing(6);

    m_stageAllButton = new QPushButton("Stage all");
    m_stageAllButton->setMaximumWidth(70);
    m_stageSelectionButton = new QPushButton("Stage");
    m_stageSelectionButton->setMaximumWidth(56);
    m_unstageSelectionButton = new QPushButton("Unstage");
    m_unstageSelectionButton->setMaximumWidth(64);
    
    actionRow->addWidget(m_stageAllButton);
    actionRow->addWidget(m_stageSelectionButton);
    actionRow->addWidget(m_unstageSelectionButton);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    // 4. File List
    m_statusList = new QListWidget;
    m_statusList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_statusList, 1);

    connect(m_refreshButton, &QPushButton::clicked, this, &GitPanel::refreshStatus);
    connect(m_initButton, &QPushButton::clicked, this, &GitPanel::initRepository);
    connect(m_connectGithubButton, &QPushButton::clicked, this, &GitPanel::connectToGithub);
    connect(m_stageAllButton, &QPushButton::clicked, this, &GitPanel::stageAll);
    connect(m_stageSelectionButton, &QPushButton::clicked, this, &GitPanel::stageSelected);
    connect(m_unstageSelectionButton, &QPushButton::clicked, this, &GitPanel::unstageSelected);
    connect(m_commitButton, &QPushButton::clicked, this, &GitPanel::commitChanges);
    connect(m_commitInput, &QLineEdit::returnPressed, this, &GitPanel::commitChanges);
    connect(m_statusList, &QListWidget::itemActivated, this, &GitPanel::onItemActivated);
    connect(m_statusList, &QListWidget::itemClicked, this, &GitPanel::onItemActivated);

    m_gitProcess = new QProcess(this);
    m_gitProcess->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_gitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GitPanel::onGitProcessFinished);
    connect(m_gitProcess, &QProcess::errorOccurred,
            this, &GitPanel::onGitProcessError);

    m_gitTimeoutTimer = new QTimer(this);
    m_gitTimeoutTimer->setSingleShot(true);
    m_gitTimeoutTimer->setInterval(kGitTimeoutMs);
    connect(m_gitTimeoutTimer, &QTimer::timeout, this, [this]() {
        if (!m_gitProcess || m_gitProcess->state() == QProcess::NotRunning)
            return;

        m_gitProcess->kill();
        m_pendingOperation = GitOperation::Status;
        setBusy(false);
        setSummaryMessage("Timed out while running Git.", true);
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &GitPanel::applyTheme);
    applyTheme();
}

void GitPanel::setRootPath(const QString &path)
{
    m_rootPath = path;
    refreshStatus();
}

void GitPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QString bg = tm.semanticColor(ThemeManager::SemanticRole::Surface).name();
    const QString border = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString muted = tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
    const QString faint = tm.semanticColor(ThemeManager::SemanticRole::TextFaint).name();
    const QString inputBg = tm.semanticColor(ThemeManager::SemanticRole::InputBg).name();
    const QString buttonBg = tm.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
    const QString buttonHover = tm.semanticColor(ThemeManager::SemanticRole::ButtonHover).name();
    const QString accent = tm.semanticColor(ThemeManager::SemanticRole::Accent).name();
    const QString accentHover = tm.semanticColor(ThemeManager::SemanticRole::AccentHover).name();
    const QString onAccent = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    const QString hover = tm.semanticColor(ThemeManager::SemanticRole::Hover).name();
    const QString selected = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString selectedText =
        tm.semanticColor(ThemeManager::SemanticRole::SelectedText).name();
    const QString success = tm.semanticColor(ThemeManager::SemanticRole::Success).name();
    const QString info = tm.semanticColor(ThemeManager::SemanticRole::Info).name();

    for (QWidget *child : findChildren<QWidget *>())
        child->setStyleSheet({});

    setStyleSheet(QString("GitPanel { background: %1; }").arg(bg));

    m_summaryLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
                                      .arg(tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name()));

    m_branchLabel->setStyleSheet(
        QString("color: %1; border: 1px solid %2; border-radius: 4px; padding: 2px 6px; "
                "font-size: 11px; font-weight: bold; background: %3;")
            .arg(accent, border, inputBg));

    m_refreshButton->setStyleSheet(
        QString("background: transparent; color: %1; border: none; font-size: 11px;")
            .arg(accent));

    m_initButton->setStyleSheet(
        QString("background: %1; color: %2; font-weight: bold; padding: 7px; "
                "border: none; border-left: 2px solid %3; border-bottom: 2px solid %3; "
                "border-radius: 6px;")
            .arg(success, onAccent, success));

    m_connectGithubButton->setStyleSheet(
        QString("background: %1; color: %2; font-weight: bold; padding: 7px; "
                "border: none; border-left: 2px solid %3; border-bottom: 2px solid %3; "
                "border-radius: 6px;")
            .arg(info, onAccent, info));

    m_commitInput->setStyleSheet(
        QString("QLineEdit { background: %1; color: %2; border: 1px solid %3; "
                "border-radius: 6px; padding: 6px 8px; font-size: 12px; }")
            .arg(inputBg, text, border));

    m_commitButton->setStyleSheet(
        QString("QPushButton { background: %1; color: %2; font-weight: bold; border: none; "
                "border-left: 2px solid %4; border-bottom: 2px solid %4; "
                "border-radius: 6px; padding: 6px 10px; font-size: 12px; } "
                "QPushButton:hover { background: %3; }")
            .arg(accent, onAccent, accentHover, accent));

    const QString actionStyle =
        QString("QPushButton { background: %1; color: %2; border: none; border-radius: 4px; "
                "border-left: 2px solid %4; border-bottom: 2px solid %4; "
                "border-radius: 6px; padding: 5px 9px; font-size: 11px; } "
                "QPushButton:hover { background: %3; }")
            .arg(buttonBg, text, buttonHover, accent);
    m_stageAllButton->setStyleSheet(actionStyle);
    m_stageSelectionButton->setStyleSheet(actionStyle);
    m_unstageSelectionButton->setStyleSheet(actionStyle);

    m_statusList->setStyleSheet(
        QString("QListWidget { background: %1; color: %2; border: 1px solid %3; "
                "border-radius: 4px; outline: none; }"
                "QListWidget::item { border-bottom: 1px solid transparent; }"
                "QListWidget::item:selected { background: %4; }"
                "QListWidget::item:hover { background: %5; }")
            .arg(bg, text, border, selected, hover));

    for (int i = 0; i < m_statusList->count(); ++i) {
        QWidget *rowWidget = m_statusList->itemWidget(m_statusList->item(i));
        if (!rowWidget)
            continue;

        const QList<QLabel *> labels = rowWidget->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->objectName() == QStringLiteral("gitStatusName")) {
                label->setStyleSheet(
                    QString("font-size: 12px; color: %1;").arg(text));
            } else if (label->objectName() == QStringLiteral("gitStatusPath")) {
                label->setStyleSheet(
                    QString("font-size: 11px; color: %1;").arg(faint));
            } else {
                label->setStyleSheet({});
            }
        }
    }
}


void GitPanel::refreshStatus()
{
    if (m_activeOperation != GitOperation::None) {
        m_pendingOperation = GitOperation::Status;
        return;
    }

    m_statusList->clear();
    m_initButton->hide();
    m_connectGithubButton->hide();

    if (m_rootPath.isEmpty()) {
        m_branchLabel->setText("No Repo");
        setSummaryMessage("Open a project inside a Git repository.");
        return;
    }

    m_branchLabel->setText("...");
    startGitOperation(GitOperation::Status, {"status", "--short", "--branch"});
}

void GitPanel::stageAll()
{
    m_pendingFileCount = m_statusList->count();
    startGitOperation(GitOperation::Stage, {"add", "--all"});
}

void GitPanel::stageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to stage.", true);
        return;
    }

    m_pendingFileCount = relativePaths.size();
    QStringList args = {"add", "--"};
    args.append(relativePaths);
    startGitOperation(GitOperation::Stage, args);
}

void GitPanel::unstageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to unstage.", true);
        return;
    }

    m_pendingFileCount = relativePaths.size();
    QStringList args = {"restore", "--staged", "--"};
    args.append(relativePaths);
    startGitOperation(GitOperation::Unstage, args);
}

void GitPanel::commitChanges()
{
    const QString message = m_commitInput->text().trimmed();
    if (message.isEmpty()) {
        setSummaryMessage("Enter a commit message before committing.", true);
        return;
    }

    startGitOperation(GitOperation::Commit, {"commit", "-m", message});
}

void GitPanel::onItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty())
        emit fileActivated(path);
}

void GitPanel::startGitOperation(GitOperation operation, const QStringList &args)
{
    if (m_rootPath.isEmpty()) {
        setSummaryMessage("Open a project inside a Git repository.", true);
        return;
    }

    if (m_activeOperation != GitOperation::None) {
        m_pendingOperation = GitOperation::Status;
        return;
    }

    if (!m_gitProcess || !m_gitTimeoutTimer)
        return;

    m_activeOperation = operation;
    m_gitProcess->setWorkingDirectory(m_rootPath);
    m_gitProcess->start("git", args);

    setBusy(true);
    m_gitTimeoutTimer->start();
}

void GitPanel::onGitProcessError(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart)
        return;

    m_gitTimeoutTimer->stop();
    const GitOperation failedOperation = m_activeOperation;
    m_activeOperation = GitOperation::None;
    setSummaryMessage("Failed to start Git.", true);
    setBusy(false);
    if (failedOperation != GitOperation::None)
        QTimer::singleShot(0, this, &GitPanel::refreshStatus);
}

void GitPanel::onGitProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_gitProcess || m_activeOperation == GitOperation::None)
        return;

    m_gitTimeoutTimer->stop();
    const QString stdOut = QString::fromUtf8(m_gitProcess->readAllStandardOutput()).trimmed();
    const QString stdErr = QString::fromUtf8(m_gitProcess->readAllStandardError()).trimmed();
    const GitOperation finishedOperation = m_activeOperation;
    m_activeOperation = GitOperation::None;

    const bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
    handleOperationFinished(finishedOperation, stdOut, stdErr, success);

    if (m_pendingOperation != GitOperation::None) {
        const GitOperation pending = m_pendingOperation;
        m_pendingOperation = GitOperation::None;
        if (pending == GitOperation::Status) {
            refreshStatus();
        }
    } else if (finishedOperation != GitOperation::Status
               && finishedOperation != GitOperation::Remote) {
        QTimer::singleShot(0, this, &GitPanel::refreshStatus);
    }
}

void GitPanel::handleOperationFinished(GitOperation operation,
                                       const QString &stdOut,
                                       const QString &stdErr,
                                       bool success)
{
    switch (operation) {
    case GitOperation::Status:
        m_branchLabel->setText("No Repo");
        if (!success) {
            setSummaryMessage(stdErr.isEmpty()
                ? "Current workspace is not a Git repository."
                : stdErr,
                true);
            m_initButton->show();
            break;
        }

        loadStatusOutput(stdOut);
        startGitOperation(GitOperation::Remote, {"remote"});
        break;

    case GitOperation::Remote:
        if (success && stdOut.contains("origin"))
            m_connectGithubButton->hide();
        else
            m_connectGithubButton->show();
        break;

    case GitOperation::Stage:
        if (success) {
            const int count = m_pendingFileCount > 0 ? m_pendingFileCount
                                                     : m_statusList->count();
            setSummaryMessage(QString("Staged %1 file(s).").arg(count));
        } else {
            setSummaryMessage(stdErr.isEmpty() ? "Failed to stage files." : stdErr, true);
        }
        m_pendingFileCount = 0;
        break;

    case GitOperation::Unstage:
        if (success) {
            setSummaryMessage(QString("Unstaged %1 file(s).").arg(m_pendingFileCount));
        } else {
            setSummaryMessage(stdErr.isEmpty() ? "Failed to unstage selected files." : stdErr,
                              true);
        }
        m_pendingFileCount = 0;
        break;

    case GitOperation::Commit:
        if (success) {
            m_commitInput->clear();
            setSummaryMessage(stdOut.simplified().isEmpty()
                ? "Commit created successfully."
                : stdOut.simplified());
        } else {
            setSummaryMessage(stdErr.isEmpty() ? "Commit failed." : stdErr, true);
        }
        break;

    case GitOperation::Init:
        setSummaryMessage(!success && !stdErr.isEmpty() ? stdErr
                                                        : "Repository initialized.",
                          !success);
        break;

    case GitOperation::ConnectToGithub:
        setSummaryMessage(!success && !stdErr.isEmpty() ? stdErr
                                                        : "Remote 'origin' added.",
                          !success);
        break;

    case GitOperation::None:
        break;
    }

    if (m_activeOperation == GitOperation::None)
        setBusy(false);
}

void GitPanel::loadStatusOutput(const QString &output)
{
    auto &tm = ThemeManager::instance();
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

        const QString statusStr = line.left(2);
        QString relativePath = line.mid(3).trimmed();
        const qsizetype renameArrow = relativePath.indexOf(" -> ");
        if (renameArrow >= 0)
            relativePath = relativePath.mid(renameArrow + 4).trimmed();

        auto *item = new QListWidgetItem();
        const QString absolutePath = QDir(m_rootPath).filePath(relativePath);
        if (QFileInfo::exists(absolutePath))
            item->setData(Qt::UserRole, absolutePath);
        item->setData(Qt::UserRole + 1, relativePath);
        item->setSizeHint(QSize(0, 26));
        m_statusList->addItem(item);

        QWidget *rowWidget = new QWidget;
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(6, 2, 6, 2);
        rowLayout->setSpacing(8);

        QLabel *badge = new QLabel(statusStr.trimmed());
        badge->setObjectName(QStringLiteral("gitStatusBadge"));
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(20, 16);

        QString badgeBg = tm.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
        QString badgeFg = tm.semanticColor(ThemeManager::SemanticRole::ButtonText).name();
        if (statusStr.contains("M")) {
            badgeBg = tm.semanticColor(ThemeManager::SemanticRole::Warning).name();
            badgeFg = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
        } else if (statusStr.contains("A")) {
            badgeBg = tm.semanticColor(ThemeManager::SemanticRole::Success).name();
            badgeFg = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
        } else if (statusStr.contains("D")) {
            badgeBg = tm.semanticColor(ThemeManager::SemanticRole::Error).name();
            badgeFg = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
        } else if (statusStr.contains("?")) {
            badgeBg = tm.semanticColor(ThemeManager::SemanticRole::Info).name();
            badgeFg = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
        }

        badge->setStyleSheet(QString("background: %1; color: %2; border-radius: 4px; font-size: 10px; font-weight: bold; font-family: monospace;").arg(badgeBg, badgeFg));
        rowLayout->addWidget(badge);

        QLabel *nameLabel = new QLabel(QFileInfo(relativePath).fileName());
        nameLabel->setObjectName(QStringLiteral("gitStatusName"));
        nameLabel->setStyleSheet("font-size: 12px;");
        rowLayout->addWidget(nameLabel);

        QLabel *pathLabel = new QLabel(QFileInfo(relativePath).path());
        pathLabel->setObjectName(QStringLiteral("gitStatusPath"));
        pathLabel->setStyleSheet("font-size: 11px;");
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
    auto &tm = ThemeManager::instance();
    m_summaryLabel->setText(message);
    m_summaryLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
        .arg(isError
                 ? tm.semanticColor(ThemeManager::SemanticRole::Error).name()
                 : tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name()));
}

void GitPanel::setBusy(bool busy)
{
    m_refreshButton->setEnabled(!busy);
    m_commitButton->setEnabled(!busy);
    m_commitInput->setEnabled(!busy);
    m_stageAllButton->setEnabled(!busy);
    m_stageSelectionButton->setEnabled(!busy);
    m_unstageSelectionButton->setEnabled(!busy);
    m_initButton->setEnabled(!busy);
    m_connectGithubButton->setEnabled(!busy);
}

void GitPanel::initRepository()
{
    startGitOperation(GitOperation::Init, {"init", "-b", "main"});
}

void GitPanel::connectToGithub()
{
    bool ok;
    QString url = QInputDialog::getText(this, "Connect to GitHub",
                                        "Enter GitHub Repository URL\(e.g., https://github.com/user/repo.git):",
                                        QLineEdit::Normal,
                                        "", &ok);
    if (!ok || url.trimmed().isEmpty())
        return;

    startGitOperation(GitOperation::ConnectToGithub,
                      {"remote", "add", "origin", url.trimmed()});
}
