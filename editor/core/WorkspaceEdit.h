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
        std::optional<int> version;
    };

    struct Operation
    {
        enum class Kind
        {
            TextDocumentEdit,
            CreateFile,
            RenameFile,
            DeleteFile
        };

        Kind kind = Kind::TextDocumentEdit;
        QString uri;
        QString newUri;
        QList<QPair<LspRange, QString>> edits;
        std::optional<int> version;
        bool ignoreIfExists = false;
        bool overwrite = false;
    };

    static std::optional<WorkspaceEdit> fromJson(
        const QJsonObject &edit,
        QString *errorMessage = nullptr);

    static std::optional<QString> applyToText(
        const QString &text,
        const QList<QPair<LspRange, QString>> &edits,
        QString *errorMessage = nullptr);

    const QList<Target> &targets() const { return m_targets; }
    const QList<Operation> &operations() const { return m_operations; }

private:
    WorkspaceEdit(QList<Target> targets, QList<Operation> operations);

    QList<Target> m_targets;
    QList<Operation> m_operations;
};

#endif
