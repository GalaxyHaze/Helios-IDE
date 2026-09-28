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
    CodeEditor *currentEditorFor(LspClient *client, const QString &uri,
                                 int version) const;
    void handleFormatting(LspClient *client, const QString &uri, int version,
                          const QList<QPair<LspRange, QString>> &edits);
    void handleDocumentSymbols(LspClient *client, const QString &uri,
                               int version,
                               const QJsonArray &symbols);
    void handleSemanticTokens(LspClient *client, const QString &uri,
                              int version,
                              const QList<LspSemanticToken> &tokens);
    void handleFoldingRanges(LspClient *client, const QString &uri,
                             int version,
                             const QList<LspFoldingRange> &ranges);

    QTabWidget *m_tabWidget;
    OutlinePanel *m_outlinePanel;
};

#endif
