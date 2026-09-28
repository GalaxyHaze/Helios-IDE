#ifndef GITSTATUSLISTPRESENTER_H
#define GITSTATUSLISTPRESENTER_H

#include "../core/GitRepositorySession.h"

#include <QObject>
#include <QStringList>

class QListWidget;

class GitStatusListPresenter : public QObject
{
    Q_OBJECT

public:
    explicit GitStatusListPresenter(QListWidget *statusList,
                                    QObject *parent = nullptr);

    void render(const GitRepositoryState &state);
    void applyTheme();
    void clear();
    [[nodiscard]] QStringList selectedRelativePaths() const;

private:
    void renderRow(const GitStatusEntry &entry,
                   const QStringList &selectedPaths);

    QListWidget *m_statusList = nullptr;
    GitRepositoryState m_state;
};

#endif
