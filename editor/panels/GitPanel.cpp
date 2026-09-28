#include "GitPanel.h"
#include "GitPanelPresentationModel.h"
#include "GitStatusListPresenter.h"
#include "../core/ThemeManager.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
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
    m_statusPresenter = new GitStatusListPresenter(m_statusList, this);

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

    m_session = new GitRepositorySession(this);
    connect(m_session, &GitRepositorySession::stateChanged,
            this, &GitPanel::onStateChanged);
    connect(m_session, &GitRepositorySession::messageChanged,
            this, &GitPanel::onSessionMessage);
    connect(m_session, &GitRepositorySession::commitSucceeded,
            this, &GitPanel::onCommitSucceeded);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &GitPanel::applyTheme);
    applyTheme();
}

void GitPanel::setRootPath(const QString &path)
{
    m_session->setRootPath(path);
}

QString GitPanel::rootPath() const
{
    return m_session ? m_session->rootPath() : QString();
}

void GitPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QString bg = tm.semanticColor(ThemeManager::SemanticRole::Surface).name();
    const QString border = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString inputBg = tm.semanticColor(ThemeManager::SemanticRole::InputBg).name();
    const QString buttonBg = tm.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
    const QString buttonHover = tm.semanticColor(ThemeManager::SemanticRole::ButtonHover).name();
    const QString accent = tm.semanticColor(ThemeManager::SemanticRole::Accent).name();
    const QString accentHover = tm.semanticColor(ThemeManager::SemanticRole::AccentHover).name();
    const QString onAccent = tm.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    const QString hover = tm.semanticColor(ThemeManager::SemanticRole::Hover).name();
    const QString selected = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
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

    if (m_statusPresenter)
        m_statusPresenter->applyTheme();
}


void GitPanel::refreshStatus()
{
    if (m_statusPresenter)
        m_statusPresenter->clear();
    m_repositoryState = {};
    m_repositoryState.rootPath = m_session->rootPath();
    m_initButton->hide();
    m_connectGithubButton->hide();
    m_branchLabel->setText("...");
    m_session->refresh();
}

void GitPanel::stageAll()
{
    m_session->stageAll();
}

void GitPanel::stageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to stage.", true);
        return;
    }

    m_session->stage(relativePaths);
}

void GitPanel::unstageSelected()
{
    const QStringList relativePaths = selectedRelativePaths();
    if (relativePaths.isEmpty()) {
        setSummaryMessage("Select one or more files to unstage.", true);
        return;
    }

    m_session->unstage(relativePaths);
}

void GitPanel::commitChanges()
{
    const QString message = m_commitInput->text().trimmed();
    if (message.isEmpty()) {
        setSummaryMessage("Enter a commit message before committing.", true);
        return;
    }

    m_session->commit(message);
}

void GitPanel::onItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty())
        emit fileActivated(path);
}

void GitPanel::onStateChanged(const GitRepositoryState &state)
{
    m_repositoryState = state;
    const GitPanelPresentationState presentation =
        GitPanelPresentationModel::fromRepositoryState(state);
    setBusy(presentation.busy);
    m_initButton->setVisible(presentation.showInitializeButton);
    m_connectGithubButton->setVisible(presentation.showConnectButton);
    m_branchLabel->setText(presentation.branchLabel);
    setSummaryMessage(presentation.summaryMessage);

    if (!presentation.showStatusList) {
        if (m_statusPresenter)
            m_statusPresenter->clear();
        return;
    }

    if (!m_statusPresenter)
        return;

    m_statusPresenter->render(m_repositoryState);
}

QStringList GitPanel::selectedRelativePaths() const
{
    return m_statusPresenter ? m_statusPresenter->selectedRelativePaths()
                             : QStringList();
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

void GitPanel::onSessionMessage(const QString &message, bool isError)
{
    setSummaryMessage(message, isError);
}

void GitPanel::onCommitSucceeded()
{
    m_commitInput->clear();
}

void GitPanel::initRepository()
{
    m_session->initializeRepository();
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

    m_session->connectToRemote(url);
}
