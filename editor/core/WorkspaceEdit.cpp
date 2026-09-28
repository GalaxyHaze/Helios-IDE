#include "WorkspaceEdit.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>

#include <algorithm>
#include <cmath>

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
}

WorkspaceEdit::WorkspaceEdit(QList<Target> targets)
    : m_targets(std::move(targets))
{
}

std::optional<WorkspaceEdit> WorkspaceEdit::fromJson(
    const QJsonObject &edit,
    QString *errorMessage)
{
    if (edit.contains(QStringLiteral("documentChanges")) ||
        !edit.value(QStringLiteral("changes")).isObject())
    {
        setError(errorMessage, QStringLiteral("Unsupported workspace edit from LSP."));
        return std::nullopt;
    }

    QList<Target> targets;
    const QJsonObject changes = edit.value(QStringLiteral("changes")).toObject();
    for (auto iterator = changes.constBegin(); iterator != changes.constEnd();
         ++iterator)
    {
        const QUrl url(iterator.key());
        if (!url.isLocalFile())
        {
            setError(errorMessage,
                     QStringLiteral("Workspace edit contains a non-local URI."));
            return std::nullopt;
        }

        if (!iterator.value().isArray())
        {
            setError(errorMessage,
                     QStringLiteral("Workspace edit contains invalid text edits."));
            return std::nullopt;
        }

        Target target;
        target.uri = iterator.key();
        const QJsonArray edits = iterator.value().toArray();
        for (const QJsonValue &value : edits)
        {
            if (!value.isObject())
            {
                setError(errorMessage,
                         QStringLiteral("Workspace edit contains invalid text edits."));
                return std::nullopt;
            }

            const QJsonObject textEdit = value.toObject();
            LspRange range;
            if (!readRange(textEdit.value(QStringLiteral("range")), &range) ||
                !textEdit.value(QStringLiteral("newText")).isString())
            {
                setError(errorMessage,
                         QStringLiteral("Workspace edit contains invalid text edits."));
                return std::nullopt;
            }

            target.edits.append({range,
                                 textEdit.value(QStringLiteral("newText")).toString()});
        }
        targets.append(std::move(target));
    }

    return WorkspaceEdit(std::move(targets));
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

    QString updated = text;
    for (const NormalizedEdit &edit : normalized)
        updated.replace(edit.start, edit.end - edit.start, edit.replacement);

    return updated;
}
