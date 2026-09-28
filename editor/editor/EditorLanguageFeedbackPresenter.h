#ifndef EDITORLANGUAGEFEEDBACKPRESENTER_H
#define EDITORLANGUAGEFEEDBACKPRESENTER_H

#include <QMetaObject>
#include <QObject>

class CodeEditor;
class LspClient;

class EditorLanguageFeedbackPresenter : public QObject
{
public:
    explicit EditorLanguageFeedbackPresenter(CodeEditor *editor,
                                             QObject *parent = nullptr);

    void attach(LspClient *client);
    void detach();

private:
    CodeEditor *m_editor = nullptr;
    LspClient *m_client = nullptr;
    QMetaObject::Connection m_diagnosticsConnection;
    QMetaObject::Connection m_hoverConnection;
    QMetaObject::Connection m_definitionConnection;
    QMetaObject::Connection m_implementationConnection;
    QMetaObject::Connection m_declarationConnection;
    QMetaObject::Connection m_signatureConnection;
    QMetaObject::Connection m_highlightsConnection;
};

#endif
