#include "CompilerPanel.h"
#include "../core/ThemeManager.h"
#include <QFont>
#include <QScrollBar>
#include <QVBoxLayout>

CompilerPanel::CompilerPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("compilerOutputDock"));

    m_output = new QPlainTextEdit;
    m_output->setReadOnly(true);
    m_output->setFont(QFont("monospace", 10));
    applyTheme();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_output);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &CompilerPanel::applyTheme);
}

CompilerPanel::~CompilerPanel() = default;

void CompilerPanel::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QColor canvas =
        tm.semanticColor(ThemeManager::SemanticRole::Canvas);
    const QColor text =
        tm.semanticColor(ThemeManager::SemanticRole::Text);
    setStyleSheet(QString("CompilerPanel { background: %1; color: %2; }")
                      .arg(canvas.name(), text.name()));
    m_output->setStyleSheet(
        QString("QPlainTextEdit { background: %1; color: %2; border: none; "
                "padding: 4px; }")
            .arg(canvas.name(), text.name()));
}

void CompilerPanel::clearOutput()
{
    m_output->clear();
}

void CompilerPanel::startBuild(const QString &title)
{
    m_output->clear();
    m_output->appendPlainText(title);
    m_output->appendPlainText(QString());

    emit compileStarted();
}

void CompilerPanel::appendOutput(const QString &text)
{
    if (!text.isEmpty()) {
        m_output->appendPlainText(text);
        auto *scroll = m_output->verticalScrollBar();
        scroll->setValue(scroll->maximum());
    }
}

void CompilerPanel::appendRawOutput(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return;
    QTextCursor cursor(m_output->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString::fromUtf8(bytes));
    auto *scroll = m_output->verticalScrollBar();
    scroll->setValue(scroll->maximum());
}

void CompilerPanel::appendDiagnostics(const QList<LspDiagnostic> &diagnostics)
{
    for (const LspDiagnostic &diagnostic : diagnostics) {
        const QString message =
            QStringLiteral("  line %1, col %2: %3")
                .arg(diagnostic.range.start.line + 1)
                .arg(diagnostic.range.start.character + 1)
                .arg(diagnostic.message);
        appendOutput(message);
    }
}

QString CompilerPanel::outputText() const
{
    return m_output->toPlainText();
}

void CompilerPanel::stopRunningTask()
{
    emit stopRequested();
}

void CompilerPanel::showBuildResult(bool success, const QString &programUri)
{
    QString msg;
    if (success && !programUri.isEmpty())
        msg = QString("Build/run result available at %1").arg(programUri);
    else if (success)
        msg = "Completed successfully";
    else
        msg = "Failed";
    m_output->appendPlainText(msg);
    emit compileFinished(success ? 0 : 1);
}
