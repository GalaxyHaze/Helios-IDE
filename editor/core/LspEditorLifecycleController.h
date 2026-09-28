#ifndef LSPEDITORLIFECYCLECONTROLLER_H
#define LSPEDITORLIFECYCLECONTROLLER_H

#include <QObject>

class QTabWidget;
class LspClient;
class LspDocumentCoordinator;

class LspEditorLifecycleController : public QObject
{
public:
    explicit LspEditorLifecycleController(
        QTabWidget *tabWidget, LspDocumentCoordinator *documentCoordinator,
        QObject *parent = nullptr);

    void openDocumentsFor(LspClient *client) const;
    void detachDocumentsFor(LspClient *client) const;

private:
    QTabWidget *m_tabWidget;
    LspDocumentCoordinator *m_documentCoordinator;
};

#endif
