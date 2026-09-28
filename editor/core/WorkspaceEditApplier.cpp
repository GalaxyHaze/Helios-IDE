#include "WorkspaceEditApplier.h"

#include "WorkspaceEdit.h"

#include "../editor/Code.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSet>
#include <QTabWidget>
#include <QUrl>

#include <utility>

namespace
{
struct FileState
{
    QString uri;
    QString text;
    CodeEditor *editor = nullptr;
    bool exists = false;
    bool needsWrite = false;
};

struct ResourceAction
{
    WorkspaceEdit::Operation::Kind kind;
    QString uri;
    QString newUri;
    bool ignoreIfExists = false;
    bool overwrite = false;
    bool apply = true;
};

struct Backup
{
    QString path;
    bool existed = false;
    QByteArray contents;
};

QString pathForUri(const QString &uri)
{
    return QUrl(uri).toLocalFile();
}

CodeEditor *editorForUri(QTabWidget *tabWidget, const QString &uri)
{
    if (!tabWidget)
        return nullptr;

    for (int index = 0; index < tabWidget->count(); ++index)
    {
        auto *editor =
            qobject_cast<CodeEditor *>(tabWidget->widget(index));
        if (editor && editor->fileUri() == uri)
            return editor;
    }
    return nullptr;
}

bool readFile(const QString &path, QString *text, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (error)
            *error = QStringLiteral("Could not read workspace-edit target.");
        return false;
    }
    if (text)
        *text = QString::fromUtf8(file.readAll());
    return true;
}

bool writeFile(const QString &path, const QString &text, QString *error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) ||
        file.write(text.toUtf8()) < 0 || !file.commit())
    {
        if (error)
            *error = QStringLiteral("Could not write workspace-edit target.");
        return false;
    }
    return true;
}

bool captureBackup(const QString &path, QList<Backup> *backups,
                   QSet<QString> *seen, QString *error)
{
    if (!backups || !seen || path.isEmpty() || seen->contains(path))
        return true;
    seen->insert(path);

    Backup backup;
    backup.path = path;
    backup.existed = QFileInfo::exists(path);
    if (backup.existed)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            if (error)
                *error = QStringLiteral(
                    "Could not back up workspace-edit target.");
            return false;
        }
        backup.contents = file.readAll();
    }
    backups->append(std::move(backup));
    return true;
}

void restoreBackups(const QList<Backup> &backups)
{
    for (const Backup &backup : backups)
        QFile::remove(backup.path);

    for (const Backup &backup : backups)
    {
        if (!backup.existed)
            continue;

        QSaveFile file(backup.path);
        if (file.open(QIODevice::WriteOnly) &&
            file.write(backup.contents) >= 0)
            file.commit();
    }
}
}

WorkspaceEditApplier::WorkspaceEditApplier(QTabWidget *tabWidget,
                                           Callbacks callbacks,
                                           QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget),
      m_callbacks(std::move(callbacks))
{
}

WorkspaceEditApplier::Result
WorkspaceEditApplier::apply(const QJsonObject &edit) const
{
    QString errorMessage;
    const auto parsed = WorkspaceEdit::fromJson(edit, &errorMessage);
    if (!parsed)
        return {false, errorMessage};

    QList<FileState> states;
    QList<ResourceAction> resources;
    QList<Backup> backups;
    QSet<QString> backedUpPaths;

    const auto stateFor = [&states, this](const QString &uri,
                                          QString *error) -> FileState * {
        for (FileState &state : states) {
            if (state.uri == uri)
                return &state;
        }

        FileState state;
        state.uri = uri;
        state.editor = editorForUri(m_tabWidget, uri);
        const QString path = pathForUri(uri);
        if (state.editor) {
            state.exists = true;
            state.text = state.editor->toPlainText();
        }
        else if (QFileInfo::exists(path)) {
            state.exists = true;
            if (!readFile(path, &state.text, error))
                return nullptr;
        }
        states.append(std::move(state));
        return &states.last();
    };

    states.reserve(parsed->operations().size());
    const auto resourceError = [&errorMessage](const QString &message) {
        errorMessage = message;
        return WorkspaceEditApplier::Result{false, errorMessage};
    };

    for (const WorkspaceEdit::Operation &operation : parsed->operations())
    {
        if (!captureBackup(pathForUri(operation.uri), &backups,
                           &backedUpPaths, &errorMessage))
            return {false, errorMessage};
        if (!operation.newUri.isEmpty() &&
            !captureBackup(pathForUri(operation.newUri), &backups,
                           &backedUpPaths, &errorMessage))
            return {false, errorMessage};

        switch (operation.kind)
        {
        case WorkspaceEdit::Operation::Kind::TextDocumentEdit: {
            FileState *state = stateFor(operation.uri, &errorMessage);
            if (!state)
                return {false, errorMessage};
            if (!state->exists)
                return resourceError(
                    QStringLiteral("Could not read workspace-edit target."));
            if (operation.version && state->editor &&
                state->editor->documentVersion() != *operation.version)
                return resourceError(QStringLiteral(
                    "Workspace edit targets an outdated document version."));

            const auto updated = WorkspaceEdit::applyToText(
                state->text, operation.edits, &errorMessage);
            if (!updated)
                return resourceError(
                    errorMessage.isEmpty()
                        ? QStringLiteral(
                              "Workspace edit contains an invalid range; no "
                              "files changed.")
                        : errorMessage);
            state->text = *updated;
            state->needsWrite = state->needsWrite || !state->editor;
            break;
        }
        case WorkspaceEdit::Operation::Kind::CreateFile: {
            FileState *state = stateFor(operation.uri, &errorMessage);
            if (!state)
                return {false, errorMessage};
            if (state->exists && operation.ignoreIfExists)
            {
                resources.append({operation.kind, operation.uri, {},
                                  operation.ignoreIfExists,
                                  operation.overwrite, false});
                break;
            }
            if (state->exists && !operation.overwrite)
                return resourceError(
                    QStringLiteral("Workspace edit cannot create an existing "
                                   "file."));
            state->exists = true;
            state->text.clear();
            state->needsWrite = !state->editor;
            resources.append({operation.kind, operation.uri, {},
                              operation.ignoreIfExists, operation.overwrite,
                              true});
            break;
        }
        case WorkspaceEdit::Operation::Kind::RenameFile: {
            if (operation.uri == operation.newUri)
                break;

            FileState *oldState = stateFor(operation.uri, &errorMessage);
            FileState *newState = stateFor(operation.newUri, &errorMessage);
            if (!oldState || !newState)
                return {false, errorMessage};
            if (!oldState->exists)
            {
                if (operation.ignoreIfExists)
                {
                    resources.append({operation.kind, operation.uri,
                                      operation.newUri,
                                      operation.ignoreIfExists,
                                      operation.overwrite, false});
                    break;
                }
                return resourceError(
                    QStringLiteral("Workspace edit cannot rename a missing "
                                   "file."));
            }
            if (newState->exists && !operation.overwrite)
                return resourceError(
                    QStringLiteral("Workspace edit cannot overwrite the "
                                   "rename target."));
            if (newState->editor && newState->editor != oldState->editor)
                return resourceError(QStringLiteral(
                    "Workspace edit targets an open rename destination."));

            newState->exists = true;
            newState->text = oldState->text;
            newState->editor = oldState->editor;
            newState->needsWrite =
                oldState->needsWrite || !newState->editor;
            oldState->exists = false;
            oldState->text.clear();
            oldState->editor = nullptr;
            oldState->needsWrite = false;
            resources.append({operation.kind, operation.uri,
                              operation.newUri, operation.ignoreIfExists,
                              operation.overwrite, true});
            break;
        }
        case WorkspaceEdit::Operation::Kind::DeleteFile: {
            FileState *state = stateFor(operation.uri, &errorMessage);
            if (!state)
                return {false, errorMessage};
            if (!state->exists)
            {
                if (operation.ignoreIfExists)
                {
                    resources.append({operation.kind, operation.uri, {},
                                      operation.ignoreIfExists,
                                      operation.overwrite, false});
                    break;
                }
                return resourceError(
                    QStringLiteral("Workspace edit cannot delete a missing "
                                   "file."));
            }
            state->exists = false;
            state->text.clear();
            state->needsWrite = false;
            resources.append({operation.kind, operation.uri, {},
                              operation.ignoreIfExists, operation.overwrite,
                              true});
            break;
        }
        }
    }

    for (const FileState &state : states)
    {
        if (!state.editor)
            continue;
        if (!state.exists && !m_callbacks.closeEditor)
            return resourceError(
                QStringLiteral("Workspace edit cannot close an open file."));
        if (state.exists && state.editor->fileUri() != state.uri &&
            !m_callbacks.renameEditor)
            return resourceError(
                QStringLiteral("Workspace edit cannot rename an open file."));
    }

    const auto failAndRestore = [&backups](const QString &message) {
        restoreBackups(backups);
        return WorkspaceEditApplier::Result{false, message};
    };

    for (const ResourceAction &resource : resources)
    {
        if (!resource.apply)
            continue;

        const QString path = pathForUri(resource.uri);
        if (resource.kind == WorkspaceEdit::Operation::Kind::CreateFile)
        {
            if (QFileInfo::exists(path) && !QFile::remove(path))
                return failAndRestore(
                    QStringLiteral("Could not create workspace-edit file."));
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly))
                return failAndRestore(
                    QStringLiteral("Could not create workspace-edit file."));
            file.close();
        }
        else if (resource.kind ==
                 WorkspaceEdit::Operation::Kind::RenameFile)
        {
            const QString newPath = pathForUri(resource.newUri);
            if (QFileInfo::exists(newPath) && resource.overwrite &&
                !QFile::remove(newPath))
                return failAndRestore(
                    QStringLiteral("Could not replace workspace-edit target."));
            if (QFileInfo::exists(path) && !QFile::rename(path, newPath))
                return failAndRestore(
                    QStringLiteral("Could not rename workspace-edit target."));
        }
        else if (resource.kind == WorkspaceEdit::Operation::Kind::DeleteFile &&
                 QFileInfo::exists(path) && !QFile::remove(path))
        {
            return failAndRestore(
                QStringLiteral("Could not delete workspace-edit target."));
        }
    }

    for (const FileState &state : states)
    {
        if (!state.exists || state.editor || !state.needsWrite)
            continue;
        if (!writeFile(pathForUri(state.uri), state.text, &errorMessage))
            return failAndRestore(errorMessage);
    }

    for (FileState &state : states)
    {
        if (!state.editor)
            continue;
        if (!state.exists)
        {
            if (!m_callbacks.closeEditor(state.editor))
                return failAndRestore(
                    QStringLiteral("Could not close deleted workspace file."));
            state.editor = nullptr;
            continue;
        }
        if (state.editor->fileUri() != state.uri &&
            !m_callbacks.renameEditor(state.editor, pathForUri(state.uri)))
            return failAndRestore(
                QStringLiteral("Could not rename open workspace file."));
        if (state.editor->toPlainText() != state.text)
        {
            state.editor->setPlainText(state.text);
            state.editor->document()->setModified(true);
        }
    }

    return {true, {}};
}
