#ifndef WORKSPACEEDITAPPLIER_H
#define WORKSPACEEDITAPPLIER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class QTabWidget;

class WorkspaceEditApplier : public QObject
{
public:
    struct Result
    {
        bool applied = false;
        QString error;
    };

    explicit WorkspaceEditApplier(QTabWidget *tabWidget,
                                  QObject *parent = nullptr);

    Result apply(const QJsonObject &edit) const;

private:
    QTabWidget *m_tabWidget;
};

#endif
