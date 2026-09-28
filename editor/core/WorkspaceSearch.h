#ifndef WORKSPACESEARCH_H
#define WORKSPACESEARCH_H

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

#include "../editor/LspClient.h"

namespace WorkspaceSearch
{
using TextEdit = QPair<LspRange, QString>;

struct ScanPolicy
{
    QStringList textExtensions;
    QStringList excludedDirs;
};

struct ReplacementRequest
{
    QString needle;
    QString replacement;
};

struct SearchReplaceTarget
{
    QString path;
    int matches = 0;
};

bool shouldScanFile(const QString &path, const ScanPolicy &policy);

QList<TextEdit> replaceEdits(const QString &text,
                             const ReplacementRequest &request);

QString applyReplaceEdits(const QString &text, const QList<TextEdit> &edits);

int offsetForPosition(const QString &text, const LspPosition &position);
}

Q_DECLARE_METATYPE(WorkspaceSearch::SearchReplaceTarget)

#endif
