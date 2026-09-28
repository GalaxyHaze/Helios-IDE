#include "LocationNavigator.h"

#include "../editor/Code.h"
#include "../panels/DiagnosticsPanel.h"
#include "../panels/OutlinePanel.h"
#include "../panels/ReferencesPanel.h"
#include "../panels/SearchPanel.h"

#include <QUrl>
#include <utility>

LocationNavigator::LocationNavigator(OpenFile openFile,
                                     CurrentEditor currentEditor,
                                     QObject *parent)
    : QObject(parent),
      m_openFile(std::move(openFile)),
      m_currentEditor(std::move(currentEditor))
{
}

bool LocationNavigator::navigateToPath(const QString &path, int line,
                                        int character)
{
    if (path.isEmpty() || !m_openFile)
        return false;

    m_openFile(path);
    auto *editor = m_currentEditor ? m_currentEditor() : nullptr;
    if (!editor)
        return false;

    editor->goToLine(line, character);
    return true;
}

bool LocationNavigator::navigateToUri(const QString &uri, int line,
                                      int character)
{
    const QUrl url(uri);
    const QString path = url.isLocalFile()
                             ? url.toLocalFile()
                             : (url.scheme().isEmpty() ? uri : QString());
    return navigateToPath(path, line, character);
}

void LocationNavigator::attach(CodeEditor *editor)
{
    if (!editor)
        return;

    connect(editor, &CodeEditor::navigateToLocation, this,
            [this](const QString &uri, int line, int character) {
                navigateToUri(uri, line, character);
            });
}

void LocationNavigator::attach(DiagnosticsPanel *panel)
{
    if (!panel)
        return;

    connect(panel, &DiagnosticsPanel::navigateToLocation, this,
            [this](const QString &uri, int line, int character) {
                navigateToUri(uri, line, character);
            });
}

void LocationNavigator::attach(OutlinePanel *panel)
{
    if (!panel)
        return;

    connect(panel, &OutlinePanel::symbolSelected, this,
            [this](int line, int character) {
                auto *editor = m_currentEditor ? m_currentEditor() : nullptr;
                if (editor)
                    editor->goToLine(line, character);
            });
}

void LocationNavigator::attach(ReferencesPanel *panel)
{
    if (!panel)
        return;

    connect(panel, &ReferencesPanel::navigateToLocation, this,
            [this](const QString &uri, int line, int character) {
                navigateToUri(uri, line, character);
            });
}

void LocationNavigator::attach(SearchPanel *panel)
{
    if (!panel)
        return;

    connect(panel, &SearchPanel::fileActivated, this,
            [this](const QString &path, int line, int character) {
                navigateToPath(path, line, character);
            });
}
