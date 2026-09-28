#ifndef LSPREFERENCESROUTER_H
#define LSPREFERENCESROUTER_H

#include "../editor/LspClient.h"

#include <QObject>
#include <functional>
#include <utility>

class QTabWidget;
class CodeEditor;
class ReferencesPanel;

class LspReferencesRouter : public QObject
{
public:
    struct Dependencies
    {
        QTabWidget *tabWidget = nullptr;
        ReferencesPanel *referencesPanel = nullptr;
    };

    explicit LspReferencesRouter(Dependencies dependencies,
                                 std::function<void()> showReferences,
                                 QObject *parent = nullptr);

    void attach(LspClient *client);

private:
    CodeEditor *currentEditorFor(const QString &uri, int version) const;
    void handleReferences(const QString &uri, int version,
                          const QList<LspLocation> &locations);

    QTabWidget *m_tabWidget = nullptr;
    ReferencesPanel *m_referencesPanel = nullptr;
    std::function<void()> m_showReferences;
};

#endif
