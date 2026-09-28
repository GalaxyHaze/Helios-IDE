#include "WorkspaceEditApplier.h"

#include "WorkspaceEdit.h"

#include "../editor/Code.h"

#include <QFile>
#include <QSaveFile>
#include <QTabWidget>
#include <QUrl>

namespace
{
struct Target
{
    QString uri;
    QString text;
    QList<QPair<LspRange, QString>> edits;
    CodeEditor *editor = nullptr;
    QString updatedText;
};
}

WorkspaceEditApplier::WorkspaceEditApplier(QTabWidget *tabWidget,
                                           QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget)
{
}

WorkspaceEditApplier::Result
WorkspaceEditApplier::apply(const QJsonObject &edit) const
{
    QString errorMessage;
    const auto parsed = WorkspaceEdit::fromJson(edit, &errorMessage);
    if (!parsed)
        return {false, errorMessage};

    QList<Target> targets;
    targets.reserve(parsed->targets().size());

    for (const WorkspaceEdit::Target &parsedTarget : parsed->targets()) {
        Target target;
        target.uri = parsedTarget.uri;
        target.edits = parsedTarget.edits;
        const QUrl url(target.uri);

        if (m_tabWidget) {
            for (int index = 0; index < m_tabWidget->count(); ++index) {
                auto *candidate =
                    qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
                if (candidate && candidate->fileUri() == target.uri) {
                    target.editor = candidate;
                    target.text = candidate->toPlainText();
                    break;
                }
            }
        }

        if (!target.editor) {
            QFile file(url.toLocalFile());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return {false, QStringLiteral(
                                   "Could not read workspace-edit target.")};
            target.text = QString::fromUtf8(file.readAll());
        }

        errorMessage.clear();
        const auto updatedText =
            WorkspaceEdit::applyToText(target.text, target.edits,
                                       &errorMessage);
        if (!updatedText) {
            return {false,
                    errorMessage.isEmpty()
                        ? QStringLiteral(
                              "Workspace edit contains an invalid range; no "
                              "files changed.")
                        : errorMessage};
        }
        target.updatedText = *updatedText;
        targets.append(std::move(target));
    }

    for (Target &target : targets) {
        if (target.editor) {
            target.editor->applyEdits(target.edits);
            target.editor->document()->setModified(true);
            continue;
        }

        QSaveFile file(QUrl(target.uri).toLocalFile());
        if (!file.open(QIODevice::WriteOnly) ||
            file.write(target.updatedText.toUtf8()) < 0 || !file.commit()) {
            return {false, QStringLiteral(
                              "Could not write workspace-edit target.")};
        }
    }

    return {true, {}};
}
