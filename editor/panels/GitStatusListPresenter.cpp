#include "GitStatusListPresenter.h"

#include "../core/ThemeManager.h"

#include <QAbstractItemView>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>

GitStatusListPresenter::GitStatusListPresenter(QListWidget *statusList,
                                               QObject *parent)
    : QObject(parent), m_statusList(statusList)
{
}

void GitStatusListPresenter::render(const GitRepositoryState &state)
{
    if (!m_statusList)
        return;

    const QStringList selectedPaths = selectedRelativePaths();
    m_state = state;
    m_statusList->clear();

    for (const GitStatusEntry &entry : m_state.entries)
        renderRow(entry, selectedPaths);
}

void GitStatusListPresenter::applyTheme()
{
    render(m_state);
}

void GitStatusListPresenter::clear()
{
    m_state = {};
    if (m_statusList)
        m_statusList->clear();
}

QStringList GitStatusListPresenter::selectedRelativePaths() const
{
    if (!m_statusList)
        return {};

    QStringList relativePaths;
    const QList<QListWidgetItem *> items = m_statusList->selectedItems();
    for (QListWidgetItem *item : items) {
        const QString relativePath =
            item->data(Qt::UserRole + 1).toString();
        if (!relativePath.isEmpty())
            relativePaths << relativePath;
    }
    relativePaths.removeDuplicates();
    return relativePaths;
}

void GitStatusListPresenter::renderRow(const GitStatusEntry &entry,
                                       const QStringList &selectedPaths)
{
    auto &theme = ThemeManager::instance();
    const QString relativePath = entry.relativePath;
    const QString status = entry.status;

    auto *item = new QListWidgetItem;
    const QString absolutePath = QDir(m_state.rootPath).filePath(relativePath);
    if (QFileInfo::exists(absolutePath))
        item->setData(Qt::UserRole, absolutePath);
    item->setData(Qt::UserRole + 1, relativePath);
    item->setSizeHint(QSize(0, 26));
    m_statusList->addItem(item);
    if (selectedPaths.contains(relativePath))
        item->setSelected(true);

    auto *rowWidget = new QWidget;
    auto *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(6, 2, 6, 2);
    rowLayout->setSpacing(8);

    auto *badge = new QLabel(status.trimmed());
    badge->setObjectName(QStringLiteral("gitStatusBadge"));
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedSize(20, 16);

    QString badgeBackground =
        theme.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
    QString badgeForeground =
        theme.semanticColor(ThemeManager::SemanticRole::ButtonText).name();
    if (status.contains(QStringLiteral("M"))) {
        badgeBackground =
            theme.semanticColor(ThemeManager::SemanticRole::Warning).name();
        badgeForeground =
            theme.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    } else if (status.contains(QStringLiteral("A"))) {
        badgeBackground =
            theme.semanticColor(ThemeManager::SemanticRole::Success).name();
        badgeForeground =
            theme.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    } else if (status.contains(QStringLiteral("D"))) {
        badgeBackground =
            theme.semanticColor(ThemeManager::SemanticRole::Error).name();
        badgeForeground =
            theme.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    } else if (status.contains(QStringLiteral("?"))) {
        badgeBackground =
            theme.semanticColor(ThemeManager::SemanticRole::Info).name();
        badgeForeground =
            theme.semanticColor(ThemeManager::SemanticRole::OnAccent).name();
    }

    badge->setStyleSheet(
        QStringLiteral("background: %1; color: %2; border-radius: 4px; "
                       "font-size: 10px; font-weight: bold; "
                       "font-family: monospace;")
            .arg(badgeBackground, badgeForeground));
    rowLayout->addWidget(badge);

    auto *nameLabel = new QLabel(QFileInfo(relativePath).fileName());
    nameLabel->setObjectName(QStringLiteral("gitStatusName"));
    nameLabel->setStyleSheet(
        QStringLiteral("font-size: 12px; color: %1;")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::Text).name()));
    rowLayout->addWidget(nameLabel);

    auto *pathLabel = new QLabel(QFileInfo(relativePath).path());
    pathLabel->setObjectName(QStringLiteral("gitStatusPath"));
    pathLabel->setStyleSheet(
        QStringLiteral("font-size: 11px; color: %1;")
            .arg(theme.semanticColor(ThemeManager::SemanticRole::TextFaint)
                     .name()));
    rowLayout->addWidget(pathLabel, 1);

    m_statusList->setItemWidget(item, rowWidget);
}
