#ifndef EDITORCHROMECONTROLLER_H
#define EDITORCHROMECONTROLLER_H

#include <QObject>

#include <functional>

class BreadcrumbsBar;
class CodeEditor;
class FindReplaceBar;
class OutlinePanel;
class QTimer;

class EditorChromeController : public QObject
{
public:
    using LanguageLabel = std::function<QString(const QString &)>;
    using CurrentEditor = std::function<CodeEditor *()>;
    using RequestOutline =
        std::function<void(const QString &uri, int version)>;
    using SetEditorPosition = std::function<void(int line, int column)>;
    using SetLanguage = std::function<void(const QString &)>;
    using SetWindowTitle = std::function<void(const QString &)>;

    explicit EditorChromeController(
        BreadcrumbsBar *breadcrumbs, FindReplaceBar *findReplaceBar,
        OutlinePanel *outlinePanel, LanguageLabel languageLabelForPath,
        CurrentEditor currentEditor, RequestOutline requestOutline,
        SetEditorPosition setEditorPosition, SetLanguage setLanguage,
        SetWindowTitle setWindowTitle, QObject *parent = nullptr);

    void update(CodeEditor *editor);

private:
    void clear();

    BreadcrumbsBar *m_breadcrumbs;
    FindReplaceBar *m_findReplaceBar;
    OutlinePanel *m_outlinePanel;
    LanguageLabel m_languageLabelForPath;
    CurrentEditor m_currentEditor;
    RequestOutline m_requestOutline;
    SetEditorPosition m_setEditorPosition;
    SetLanguage m_setLanguage;
    SetWindowTitle m_setWindowTitle;
    QString m_outlineUri;
    int m_outlineVersion = -1;
    QString m_requestedUri;
    int m_requestedVersion = -1;
    QTimer *m_outlineTimer;
};

#endif
