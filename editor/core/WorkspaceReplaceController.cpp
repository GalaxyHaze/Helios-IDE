#include "WorkspaceReplaceController.h"

#include "../editor/Code.h"

#include <QFile>
#include <QSaveFile>

#include <utility>

WorkspaceReplaceController::WorkspaceReplaceController(Callbacks callbacks,
                                                       QObject *parent)
    : QObject(parent), m_callbacks(std::move(callbacks))
{
}

void WorkspaceReplaceController::replaceAll(
    const QString &needle, const QString &replacement,
    const QVector<WorkspaceSearch::SearchReplaceTarget> &targets)
{
    if (targets.isEmpty()) {
        showStatus(QStringLiteral("No matches for \"%1\".").arg(needle), 5000);
        return;
    }

    int totalMatches = 0;
    for (const auto &target : targets)
        totalMatches += target.matches;

    if (m_callbacks.confirmReplacement &&
        !m_callbacks.confirmReplacement(
            {totalMatches, static_cast<int>(targets.size()), needle,
             replacement}))
        return;

    int replaced = 0;
    for (const auto &target : targets) {
        CodeEditor *openEditor = m_callbacks.findOpenEditor
                                     ? m_callbacks.findOpenEditor(target.path)
                                     : nullptr;
        if (openEditor) {
            const QList<WorkspaceSearch::TextEdit> edits =
                WorkspaceSearch::replaceEdits(
                    openEditor->toPlainText(), {needle, replacement});
            if (edits.isEmpty())
                continue;

            openEditor->applyEdits(edits);
            openEditor->document()->setModified(true);
            openEditor->flushPendingLspChanges();
            replaced += edits.size();
            continue;
        }

        QFile file(target.path);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        const QString text = QString::fromUtf8(file.readAll());
        file.close();

        const QList<WorkspaceSearch::TextEdit> edits =
            WorkspaceSearch::replaceEdits(text, {needle, replacement});
        if (edits.isEmpty())
            continue;

        QSaveFile output(target.path);
        if (!output.open(QIODevice::WriteOnly)) {
            showStatus(QStringLiteral("Could not write ") + target.path, 5000);
            continue;
        }
        output.write(WorkspaceSearch::applyReplaceEdits(text, edits).toUtf8());
        if (!output.commit()) {
            showStatus(QStringLiteral("Could not save ") + target.path, 5000);
            continue;
        }
        replaced += edits.size();
    }

    showStatus(QStringLiteral("Replaced %1 matches in workspace.").arg(replaced),
               8000);
    if (m_callbacks.refreshSearch)
        m_callbacks.refreshSearch();
}

void WorkspaceReplaceController::showStatus(const QString &message,
                                             int timeout) const
{
    if (m_callbacks.showStatus)
        m_callbacks.showStatus(message, timeout);
}
