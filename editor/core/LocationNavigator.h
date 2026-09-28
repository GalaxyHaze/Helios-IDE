#ifndef LOCATIONNAVIGATOR_H
#define LOCATIONNAVIGATOR_H

#include <QObject>

#include <functional>

class CodeEditor;
class DiagnosticsPanel;
class OutlinePanel;
class ReferencesPanel;
class SearchPanel;

class LocationNavigator : public QObject
{
public:
    using OpenFile = std::function<void(const QString &)>;
    using CurrentEditor = std::function<CodeEditor *()>;

    explicit LocationNavigator(OpenFile openFile,
                               CurrentEditor currentEditor,
                               QObject *parent = nullptr);

    bool navigateToPath(const QString &path, int line, int character);
    bool navigateToUri(const QString &uri, int line, int character);

    void attach(CodeEditor *editor);
    void attach(DiagnosticsPanel *panel);
    void attach(OutlinePanel *panel);
    void attach(ReferencesPanel *panel);
    void attach(SearchPanel *panel);

private:
    OpenFile m_openFile;
    CurrentEditor m_currentEditor;
};

#endif
