#include "EditorTabCloseController.h"

#include "../editor/Code.h"

#include <QTabWidget>
#include <utility>

EditorTabCloseController::EditorTabCloseController(
    QTabWidget *tabWidget, Callbacks callbacks, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget), m_callbacks(std::move(callbacks))
{
    if (!m_tabWidget)
        return;

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this,
            [this](int index) { closeTab(index); });
}

void EditorTabCloseController::closeTab(int index)
{
    if (!m_tabWidget || index < 0 || index >= m_tabWidget->count())
        return;

    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
    if (!editor)
        return;

    if (editor->document()->isModified()) {
        if (!m_callbacks.requestSaveDecision)
            return;

        const SaveDecision decision = m_callbacks.requestSaveDecision(editor);
        if (decision == SaveDecision::Cancel)
            return;
        if (decision == SaveDecision::Save) {
            if (!m_callbacks.saveEditor ||
                !m_callbacks.saveEditor(editor) ||
                editor->document()->isModified())
                return;
        }
    }

    if (m_callbacks.releaseEditor)
        m_callbacks.releaseEditor(editor);
}
