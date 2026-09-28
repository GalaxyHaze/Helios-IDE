#include "WorkspaceEdit.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
        *errorMessage = message;
}

bool readPosition(const QJsonValue &value, LspPosition *position)
{
    if (!position || !value.isObject())
        return false;

    const QJsonObject object = value.toObject();
    const QJsonValue line = object.value(QStringLiteral("line"));
    const QJsonValue character = object.value(QStringLiteral("character"));
    if (!line.isDouble() || !character.isDouble())
        return false;

    const double lineValue = line.toDouble();
    const double characterValue = character.toDouble();
    if (lineValue < 0 || characterValue < 0 ||
        std::floor(lineValue) != lineValue ||
        std::floor(characterValue) != characterValue)
        return false;

    position->line = line.toInt();
    position->character = character.toInt();
    return true;
}

bool readRange(const QJsonValue &value, LspRange *range)
{
    if (!range || !value.isObject())
        return false;

    const QJsonObject object = value.toObject();
    return readPosition(object.value(QStringLiteral("start")), &range->start) &&
           readPosition(object.value(QStringLiteral("end")), &range->end);
}

qsizetype offsetForPosition(const QString &text,
                            const LspPosition &position,
                            bool *valid)
{
    if (position.line < 0 || position.character < 0)
    {
        *valid = false;
        return 0;
    }

    int line = 0;
    qsizetype offset = 0;
    while (line < position.line && offset < text.size())
    {
        const qsizetype newline = text.indexOf(QLatin1Char('\n'), offset);
        if (newline < 0)
        {
            *valid = false;
            return 0;
        }
        offset = newline + 1;
        ++line;
    }

    if (line != position.line)
    {
        *valid = false;
        return 0;
    }

    const qsizetype end = text.indexOf(QLatin1Char('\n'), offset);
    const qsizetype lineEnd = end < 0 ? text.size() : end;
    if (offset + position.character > lineEnd)
    {
        *valid = false;
        return 0;
    }

    return offset + position.character;
}

struct NormalizedEdit
{
    qsizetype start = 0;
    qsizetype end = 0;
    QString replacement;
};

bool readVersion(const QJsonObject &object, std::optional<int> *version)
{
    if (!version)
        return false;

    const QJsonValue value = object.value(QStringLiteral("version"));
    if (!object.contains(QStringLiteral("version")) || value.isNull())
    {
        version->reset();
        return true;
    }

    if (!value.isDouble())
        return false;

    const double number = value.toDouble();
    if (number < 0 || std::floor(number) != number ||
        number > std::numeric_limits<int>::max())
        return false;

    *version = value.toInt();
    return true;
}

bool readLocalUri(const QString &uri)
{
    const QUrl url(uri);
    return url.isLocalFile() && !url.toLocalFile().isEmpty();
}

bool readTextEdits(const QJsonValue &value,
                   QList<QPair<LspRange, QString>> *edits)
{
    if (!edits || !value.isArray())
        return false;

    for (const QJsonValue &entry : value.toArray())
    {
        if (!entry.isObject())
            return false;

        const QJsonObject textEdit = entry.toObject();
        LspRange range;
        if (!readRange(textEdit.value(QStringLiteral("range")), &range) ||
            !textEdit.value(QStringLiteral("newText")).isString())
            return false;

        edits->append(
            {range, textEdit.value(QStringLiteral("newText")).toString()});
    }
    return true;
}

bool readResourceOptions(const QJsonObject &object, bool *ignoreIfExists,
                         bool *overwrite)
{
    if (!ignoreIfExists || !overwrite)
        return false;

    *ignoreIfExists = false;
    *overwrite = false;
    const QJsonValue optionsValue = object.value(QStringLiteral("options"));
    if (!object.contains(QStringLiteral("options")))
        return true;
    if (!optionsValue.isObject())
        return false;

    const QJsonObject options = optionsValue.toObject();
    for (const QString &name : {QStringLiteral("ignoreIfExists"),
                                QStringLiteral("overwrite")})
    {
        if (options.contains(name) && !options.value(name).isBool())
            return false;
    }
    *ignoreIfExists =
        options.value(QStringLiteral("ignoreIfExists")).toBool(false);
    *overwrite = options.value(QStringLiteral("overwrite")).toBool(false);
    return true;
}
}

WorkspaceEdit::WorkspaceEdit(QList<Target> targets,
                             QList<Operation> operations)
    : m_targets(std::move(targets)), m_operations(std::move(operations))
{
}

std::optional<WorkspaceEdit> WorkspaceEdit::fromJson(
    const QJsonObject &edit,
    QString *errorMessage)
{
    QList<Target> targets;
    QList<Operation> operations;

    const auto appendTextDocumentEdit =
        [&targets, &operations](const QString &uri,
                                const QJsonValue &editsValue,
                                const std::optional<int> &version,
                                QString *error) {
            if (!readLocalUri(uri))
            {
                setError(error, QStringLiteral(
                                    "Workspace edit contains a non-local "
                                    "URI."));
                return false;
            }
            if (!editsValue.isArray())
            {
                setError(error, QStringLiteral(
                                    "Workspace edit contains invalid text "
                                    "edits."));
                return false;
            }

            QList<QPair<LspRange, QString>> edits;
            if (!readTextEdits(editsValue, &edits))
            {
                setError(error, QStringLiteral(
                                    "Workspace edit contains invalid text "
                                    "edits."));
                return false;
            }

            WorkspaceEdit::Target target{uri, edits, version};
            WorkspaceEdit::Operation operation;
            operation.kind =
                WorkspaceEdit::Operation::Kind::TextDocumentEdit;
            operation.uri = uri;
            operation.edits = edits;
            operation.version = version;
            targets.append(target);
            operations.append(std::move(operation));
            return true;
        };

    if (edit.contains(QStringLiteral("changes")))
    {
        if (!edit.value(QStringLiteral("changes")).isObject())
        {
            setError(errorMessage,
                     QStringLiteral("Workspace edit contains invalid changes."));
            return std::nullopt;
        }

        const QJsonObject changes =
            edit.value(QStringLiteral("changes")).toObject();
        for (auto iterator = changes.constBegin();
             iterator != changes.constEnd(); ++iterator)
        {
            if (!appendTextDocumentEdit(iterator.key(), iterator.value(),
                                         std::nullopt, errorMessage))
                return std::nullopt;
        }
    }

    if (edit.contains(QStringLiteral("documentChanges")))
    {
        const QJsonValue documentChangesValue =
            edit.value(QStringLiteral("documentChanges"));
        if (!documentChangesValue.isArray())
        {
            setError(errorMessage, QStringLiteral(
                                     "Workspace edit contains invalid "
                                     "document changes."));
            return std::nullopt;
        }

        for (const QJsonValue &entry : documentChangesValue.toArray())
        {
            if (!entry.isObject())
            {
                setError(errorMessage, QStringLiteral(
                                         "Workspace edit contains invalid "
                                         "document changes."));
                return std::nullopt;
            }

            const QJsonObject object = entry.toObject();
            const QString kind =
                object.value(QStringLiteral("kind")).toString();
            if (!kind.isEmpty())
            {
                WorkspaceEdit::Operation operation;
                if (kind == QLatin1String("create"))
                {
                    operation.kind =
                        WorkspaceEdit::Operation::Kind::CreateFile;
                    operation.uri =
                        object.value(QStringLiteral("uri")).toString();
                }
                else if (kind == QLatin1String("rename"))
                {
                    operation.kind =
                        WorkspaceEdit::Operation::Kind::RenameFile;
                    operation.uri =
                        object.value(QStringLiteral("oldUri")).toString();
                    operation.newUri =
                        object.value(QStringLiteral("newUri")).toString();
                }
                else if (kind == QLatin1String("delete"))
                {
                    operation.kind =
                        WorkspaceEdit::Operation::Kind::DeleteFile;
                    operation.uri =
                        object.value(QStringLiteral("uri")).toString();
                }
                else
                {
                    setError(errorMessage, QStringLiteral(
                                             "Workspace edit contains an "
                                             "unsupported resource operation."));
                    return std::nullopt;
                }

                if (!readLocalUri(operation.uri) ||
                    ((operation.kind ==
                          WorkspaceEdit::Operation::Kind::RenameFile) &&
                     !readLocalUri(operation.newUri)) ||
                    !readResourceOptions(object, &operation.ignoreIfExists,
                                          &operation.overwrite))
                {
                    setError(errorMessage, QStringLiteral(
                                             "Workspace edit contains an "
                                             "invalid resource operation."));
                    return std::nullopt;
                }
                operations.append(std::move(operation));
                continue;
            }

            const QJsonObject textDocument =
                object.value(QStringLiteral("textDocument")).toObject();
            const QString uri =
                textDocument.value(QStringLiteral("uri")).toString();
            std::optional<int> version;
            if (textDocument.isEmpty() ||
                !readVersion(textDocument, &version) ||
                !appendTextDocumentEdit(uri, object.value("edits"), version,
                                         errorMessage))
                return std::nullopt;
        }
    }

    if (!edit.contains(QStringLiteral("changes")) &&
        !edit.contains(QStringLiteral("documentChanges")))
    {
        setError(errorMessage,
                 QStringLiteral("Workspace edit contains no changes."));
        return std::nullopt;
    }

    return WorkspaceEdit(std::move(targets), std::move(operations));
}

std::optional<QString> WorkspaceEdit::applyToText(
    const QString &text,
    const QList<QPair<LspRange, QString>> &edits,
    QString *errorMessage)
{
    QList<NormalizedEdit> normalized;
    normalized.reserve(edits.size());

    for (const auto &edit : edits)
    {
        bool startValid = true;
        bool endValid = true;
        const qsizetype start =
            offsetForPosition(text, edit.first.start, &startValid);
        const qsizetype end =
            offsetForPosition(text, edit.first.end, &endValid);
        if (!startValid || !endValid || end < start)
        {
            setError(errorMessage,
                     QStringLiteral("Workspace edit contains an invalid range."));
            return std::nullopt;
        }

        normalized.append({start, end, edit.second});
    }

    std::sort(normalized.begin(), normalized.end(),
              [](const NormalizedEdit &left, const NormalizedEdit &right) {
                  if (left.start != right.start)
                      return left.start > right.start;
                  return left.end > right.end;
              });

    for (int index = 1; index < normalized.size(); ++index)
    {
        const NormalizedEdit &later = normalized.at(index - 1);
        const NormalizedEdit &earlier = normalized.at(index);
        if (earlier.end > later.start)
        {
            setError(errorMessage,
                     QStringLiteral("Workspace edit contains overlapping "
                                    "ranges."));
            return std::nullopt;
        }
    }

    QString updated = text;
    for (const NormalizedEdit &edit : normalized)
        updated.replace(edit.start, edit.end - edit.start, edit.replacement);

    return updated;
}
