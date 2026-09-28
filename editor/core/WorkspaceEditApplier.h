#ifndef WORKSPACEEDITAPPLIER_H
#define WORKSPACEEDITAPPLIER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <functional>

class CodeEditor;
class QTabWidget;

class WorkspaceEditApplier : public QObject
{
public:
    struct Result
    {
        bool applied = false;
        QString error;
    };

    struct Callbacks
    {
        std::function<bool(CodeEditor *, const QString &)> renameEditor;
        std::function<bool(CodeEditor *)> closeEditor;
    };

    explicit WorkspaceEditApplier(QTabWidget *tabWidget,
                                  Callbacks callbacks = {},
                                  QObject *parent = nullptr);

    Result apply(const QJsonObject &edit) const;

private:
    QTabWidget *m_tabWidget;
    Callbacks m_callbacks;
};

#endif
