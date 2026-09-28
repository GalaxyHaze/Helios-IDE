#ifndef LSPEDITORRESULTROUTER_H
#define LSPEDITORRESULTROUTER_H

#include "../editor/LspClient.h"

#include <QObject>

class QTabWidget;
class CodeEditor;
class OutlinePanel;

class LspEditorResultRouter : public QObject
{
public:
    explicit LspEditorResultRouter(QTabWidget *tabWidget,
                                   OutlinePanel *outlinePanel,
                                   QObject *parent = nullptr);

    void attach(LspClient *client);

private:
    CodeEditor *currentEditorFor(const QString &uri, int version) const;
    void handleFormatting(const QString &uri, int version,
                          const QList<QPair<LspRange, QString>> &edits);
    void handleDocumentSymbols(const QString &uri, int version,
                               const QJsonArray &symbols);

    QTabWidget *m_tabWidget;
    OutlinePanel *m_outlinePanel;
};

#endif
