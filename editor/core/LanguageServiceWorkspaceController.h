#ifndef LANGUAGESERVICEWORKSPACECONTROLLER_H
#define LANGUAGESERVICEWORKSPACECONTROLLER_H

#include "LanguageIdentity.h"

#include <QObject>
#include <QString>

#include <functional>

class ClangdLifecycleCoordinator;
class CodeEditor;
class LspClient;
class LspDocumentCoordinator;
class QTabWidget;

class LanguageServiceWorkspaceController : public QObject
{
    Q_OBJECT

public:
    struct Configuration
    {
        bool lspEnabled = false;
        bool cFamilyEnabled = false;
        QString clangdPath;
        QString workspaceRoot;
    };

    struct Callbacks
    {
        std::function<Configuration()> configuration;
        std::function<void()> presentationChanged;
    };

    LanguageServiceWorkspaceController(
        QTabWidget *tabs, LspDocumentCoordinator *documentCoordinator,
        ClangdLifecycleCoordinator *clangdLifecycle, Callbacks callbacks,
        QObject *parent = nullptr);

    void refreshRouting();
    void reconcile();

    bool shouldUseForPath(const QString &path);
    LspClient *clientForLanguage(LanguageIdentity::Language language) const;
    LspClient *clientForPath(const QString &path) const;
    bool isEditorLanguage(CodeEditor *editor,
                          LanguageIdentity::Language language) const;

private:
    Configuration resolveConfiguration() const;
    void applyRouting(const Configuration &configuration);

    QTabWidget *m_tabs = nullptr;
    LspDocumentCoordinator *m_documentCoordinator = nullptr;
    ClangdLifecycleCoordinator *m_clangdLifecycle = nullptr;
    Callbacks m_callbacks;
};

#endif
