#include "WorkspaceSearch.h"

#include <QDir>
#include <QFileInfo>

namespace WorkspaceSearch
{
bool shouldScanFile(const QString &path, const ScanPolicy &policy)
{
    const QString normalized = QDir::fromNativeSeparators(path);
    for (const QString &dir : policy.excludedDirs) {
        if (dir.isEmpty())
            continue;
        if (normalized.contains(QStringLiteral("/%1/").arg(dir)) ||
            normalized.endsWith(QStringLiteral("/%1").arg(dir))) {
            return false;
        }
    }

    const QString fileName = QFileInfo(normalized).fileName().toLower();
    if (fileName.isEmpty())
        return false;

    const QString suffix = QFileInfo(normalized).suffix().toLower();
    if (!suffix.isEmpty() && policy.textExtensions.contains(suffix))
        return true;

    return policy.textExtensions.contains(fileName);
}

QList<TextEdit> replaceEdits(const QString &text,
                             const ReplacementRequest &request)
{
    QList<TextEdit> edits;
    if (request.needle.isEmpty())
        return edits;

    qsizetype offset = 0;
    while (true) {
        const qsizetype found =
            text.indexOf(request.needle, offset, Qt::CaseInsensitive);
        if (found < 0)
            break;

        LspRange range;
        const QString prefix = text.left(found);
        range.start.line = static_cast<int>(prefix.count('\n'));
        const qsizetype lineStart = prefix.lastIndexOf('\n');
        range.start.character =
            static_cast<int>(lineStart < 0 ? found : found - lineStart - 1);
        range.end.line = range.start.line;
        range.end.character =
            range.start.character + static_cast<int>(request.needle.size());

        edits.append({range, request.replacement});
        offset = found + request.needle.size();
    }
    return edits;
}

QString applyReplaceEdits(const QString &text, const QList<TextEdit> &edits)
{
    QString result = text;
    for (auto it = edits.rbegin(); it != edits.rend(); ++it) {
        const LspRange &range = it->first;
        const int start = offsetForPosition(result, range.start);
        const int end = offsetForPosition(result, range.end);
        if (start < 0 || end < start)
            continue;
        result.replace(start, end - start, it->second);
    }
    return result;
}

int offsetForPosition(const QString &text, const LspPosition &position)
{
    int line = position.line;
    qsizetype offset = 0;
    while (line > 0 && offset < text.size()) {
        const qsizetype next = text.indexOf('\n', offset);
        if (next < 0)
            return -1;
        offset = next + 1;
        --line;
    }
    const qsizetype result = offset + position.character;
    if (result > text.size())
        return -1;
    return static_cast<int>(result);
}
}
