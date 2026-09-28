#include "GitStatusParser.h"

GitStatusSnapshot GitStatusParser::parse(const QString &output)
{
    GitStatusSnapshot snapshot;
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("##"))) {
            snapshot.branch = line.mid(3).trimmed();
            continue;
        }

        if (line.size() < 3)
            continue;

        GitStatusEntry entry;
        entry.status = line.left(2);
        entry.relativePath = line.mid(3).trimmed();
        const qsizetype renameArrow =
            entry.relativePath.indexOf(QStringLiteral(" -> "));
        if (renameArrow >= 0)
            entry.relativePath =
                entry.relativePath.mid(renameArrow + 4).trimmed();
        snapshot.entries.append(entry);
    }
    return snapshot;
}
