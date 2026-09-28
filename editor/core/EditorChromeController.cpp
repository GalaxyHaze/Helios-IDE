#include "EditorChromeController.h"

#include "../editor/Code.h"
#include "../panels/OutlinePanel.h"
#include "../widgets/BreadcrumbsBar.h"
#include "../widgets/FindReplaceBar.h"

#include <QFileInfo>
#include <QTextCursor>
#include <QTimer>

EditorChromeController::EditorChromeController(
    BreadcrumbsBar *breadcrumbs, FindReplaceBar *findReplaceBar,
    OutlinePanel *outlinePanel, LanguageLabel languageLabelForPath,
    CurrentEditor currentEditor, RequestOutline requestOutline,
    SetEditorPosition setEditorPosition, SetLanguage setLanguage,
    SetWindowTitle setWindowTitle, QObject *parent)
    : QObject(parent),
      m_breadcrumbs(breadcrumbs),
      m_findReplaceBar(findReplaceBar),
      m_outlinePanel(outlinePanel),
      m_languageLabelForPath(std::move(languageLabelForPath)),
      m_currentEditor(std::move(currentEditor)),
      m_requestOutline(std::move(requestOutline)),
      m_setEditorPosition(std::move(setEditorPosition)),
      m_setLanguage(std::move(setLanguage)),
      m_setWindowTitle(std::move(setWindowTitle)),
      m_outlineTimer(new QTimer(this))
{
    m_outlineTimer->setSingleShot(true);
    m_outlineTimer->setInterval(150);
    connect(m_outlineTimer, &QTimer::timeout, this, [this]() {
        if (!m_outlinePanel || !m_outlinePanel->isVisible() ||
            m_outlineUri.isEmpty() || !m_requestOutline) {
            return;
        }

        auto *editor = m_currentEditor ? m_currentEditor() : nullptr;
        if (!editor || editor->fileUri() != m_outlineUri ||
            editor->documentVersion() != m_outlineVersion) {
            return;
        }
        m_requestOutline(m_outlineUri, m_outlineVersion);
    });
}

void EditorChromeController::update(CodeEditor *editor)
{
    if (!editor) {
        clear();
        return;
    }

    if (m_findReplaceBar)
        m_findReplaceBar->setEditor(editor);

    if (m_setLanguage && m_languageLabelForPath)
        m_setLanguage(m_languageLabelForPath(editor->filePath()));

    if (m_breadcrumbs) {
        if (editor->filePath().isEmpty())
            m_breadcrumbs->clear();
        else
            m_breadcrumbs->setPath(editor->filePath());
    }

    if (m_setEditorPosition) {
        const QTextCursor cursor = editor->textCursor();
        m_setEditorPosition(cursor.blockNumber() + 1,
                            cursor.columnNumber() + 1);
    }

    if (m_setWindowTitle) {
        m_setWindowTitle(
            editor->filePath().isEmpty()
                ? QStringLiteral("Helios")
                : QStringLiteral("%1 — Helios")
                      .arg(QFileInfo(editor->filePath()).fileName()));
    }

    m_outlineUri = editor->fileUri();
    m_outlineVersion = editor->documentVersion();
    if (!m_outlinePanel || !m_outlinePanel->isVisible())
        return;

    if (m_requestedUri.isEmpty() || m_requestedUri != m_outlineUri) {
        m_outlinePanel->clear();
        m_requestedUri = m_outlineUri;
        m_requestedVersion = m_outlineVersion;
        m_outlineTimer->start();
    } else if (m_requestedVersion != m_outlineVersion) {
        m_requestedVersion = m_outlineVersion;
        m_outlineTimer->start();
    }
}

void EditorChromeController::clear()
{
    if (m_findReplaceBar)
        m_findReplaceBar->setEditor(nullptr);
    if (m_breadcrumbs)
        m_breadcrumbs->clear();
    if (m_setEditorPosition)
        m_setEditorPosition(1, 1);
    if (m_setLanguage)
        m_setLanguage(QString());
    if (m_outlinePanel)
        m_outlinePanel->clear();
    if (m_setWindowTitle)
        m_setWindowTitle(QStringLiteral("Helios"));

    m_outlineUri.clear();
    m_outlineVersion = -1;
    m_requestedUri.clear();
    m_requestedVersion = -1;
    m_outlineTimer->stop();
}
