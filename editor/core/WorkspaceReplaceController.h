#ifndef WORKSPACEREPLACECONTROLLER_H
#define WORKSPACEREPLACECONTROLLER_H

#include "WorkspaceSearch.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <functional>

class CodeEditor;

class WorkspaceReplaceController : public QObject
{
    Q_OBJECT

public:
    using FindOpenEditor = std::function<CodeEditor *(const QString &)>;
    using ConfirmReplacement =
        std::function<bool(int, int, const QString &, const QString &)>;
    using ShowStatus = std::function<void(const QString &, int)>;
    using RefreshSearch = std::function<void()>;

    struct Callbacks
    {
        FindOpenEditor findOpenEditor;
        ConfirmReplacement confirmReplacement;
        ShowStatus showStatus;
        RefreshSearch refreshSearch;
    };

    explicit WorkspaceReplaceController(Callbacks callbacks,
                                        QObject *parent = nullptr);

    void replaceAll(
        const QString &needle, const QString &replacement,
        const QVector<WorkspaceSearch::SearchReplaceTarget> &targets);

private:
    void showStatus(const QString &message, int timeout) const;

    Callbacks m_callbacks;
};

#endif
