#ifndef LSPCOMPLETIONROUTER_H
#define LSPCOMPLETIONROUTER_H

#include "../editor/LspClient.h"

#include <QObject>

#include <functional>

class QTabWidget;
class CodeEditor;
class LspCompletionModel;
class LspCompleter;
class SnippetManager;

class LspCompletionRouter : public QObject
{
public:
    struct Dependencies
    {
        QTabWidget *tabWidget = nullptr;
        SnippetManager *snippetManager = nullptr;
        LspCompleter *completer = nullptr;
        LspCompletionModel *completionModel = nullptr;
    };

    LspCompletionRouter(Dependencies dependencies,
                        std::function<bool()> isEnabled,
                        QObject *parent = nullptr);

    void attach(LspClient *client);

private:
    void handleCompletion(LspClient *client, const QString &uri, int version,
                          const QList<LspCompletionItem> &items);

    QTabWidget *m_tabWidget;
    SnippetManager *m_snippetManager;
    LspCompleter *m_completer;
    LspCompletionModel *m_completionModel;
    std::function<bool()> m_isEnabled;
};

#endif
