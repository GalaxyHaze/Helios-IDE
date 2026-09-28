#ifndef WORKSPACEEDIT_H
#define WORKSPACEEDIT_H

#include "../editor/LspClient.h"

#include <QJsonObject>
#include <QList>
#include <QString>

#include <optional>

class WorkspaceEdit
{
public:
    struct Target
    {
        QString uri;
        QList<QPair<LspRange, QString>> edits;
    };

    static std::optional<WorkspaceEdit> fromJson(
        const QJsonObject &edit,
        QString *errorMessage = nullptr);

    static std::optional<QString> applyToText(
        const QString &text,
        const QList<QPair<LspRange, QString>> &edits,
        QString *errorMessage = nullptr);

    const QList<Target> &targets() const { return m_targets; }

private:
    explicit WorkspaceEdit(QList<Target> targets);

    QList<Target> m_targets;
};

#endif
