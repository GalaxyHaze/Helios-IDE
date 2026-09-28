#include "StatusBarController.h"

#include "ThemeManager.h"

#include <QLabel>
#include <QStatusBar>

namespace
{
QString paddedStyle(const QString &color, const QString &extra = QString())
{
    return QString("color: %1; padding: 0 4px; %2").arg(color, extra);
}
}

StatusBarController::StatusBarController(QStatusBar *statusBar, bool vimEnabled,
                                         QObject *parent)
    : QObject(parent), m_statusBar(statusBar)
{
    if (!m_statusBar)
        return;

    m_contextLabel = new QLabel(QStringLiteral("◀ 1/1 ▶"), m_statusBar);
    m_lspLabel = new QLabel(QStringLiteral("LSP ⬤"), m_statusBar);
    m_vimLabel = new QLabel(vimEnabled ? QStringLiteral("VIM: NORMAL")
                                       : QStringLiteral("VIM: OFF"),
                            m_statusBar);
    m_errorLabel = new QLabel(QStringLiteral("✕0  ⚠0"), m_statusBar);
    m_positionLabel = new QLabel(QStringLiteral("Ln 1, Col 1"), m_statusBar);
    m_indentLabel = new QLabel(QStringLiteral("Spaces: 4"), m_statusBar);
    m_encodingLabel = new QLabel(QStringLiteral("UTF-8"), m_statusBar);
    m_languageLabel = new QLabel(QStringLiteral("Zith"), m_statusBar);

    m_statusBar->addWidget(m_contextLabel);
    m_statusBar->addWidget(m_lspLabel);
    m_statusBar->addPermanentWidget(m_vimLabel);
    m_statusBar->addPermanentWidget(m_errorLabel);
    m_statusBar->addPermanentWidget(m_positionLabel);
    m_statusBar->addPermanentWidget(m_indentLabel);
    m_statusBar->addPermanentWidget(m_encodingLabel);
    m_statusBar->addPermanentWidget(m_languageLabel);

    applyTheme();
}

void StatusBarController::setContext(int index, int count)
{
    if (m_contextLabel)
        m_contextLabel->setText(QStringLiteral("%1/%2").arg(index + 1).arg(count));
}

void StatusBarController::setLspStatus(const QString &text, const QString &color)
{
    if (!m_lspLabel)
        return;

    m_lspLabel->setText(text);
    if (!color.isEmpty())
        m_lspLabelColor = color;

    const QString resolved = m_lspLabelColor.isEmpty()
                                 ? ThemeManager::instance()
                                       .semanticColor(ThemeManager::SemanticRole::Success)
                                       .name()
                                 : m_lspLabelColor;
    m_lspLabel->setStyleSheet(paddedStyle(resolved));
}

void StatusBarController::setVimMode(const QString &text)
{
    if (m_vimLabel)
        m_vimLabel->setText(text);
}

void StatusBarController::setDiagnostics(int errors, int warnings)
{
    if (m_errorLabel)
        m_errorLabel->setText(QStringLiteral("✕%1  ⚠%2").arg(errors).arg(warnings));
}

void StatusBarController::setEditorPosition(int line, int column)
{
    if (m_positionLabel)
        m_positionLabel->setText(
            QStringLiteral("Ln %1, Col %2").arg(line).arg(column));
}

void StatusBarController::setLanguage(const QString &language)
{
    if (m_languageLabel)
        m_languageLabel->setText(language);
}

void StatusBarController::applyTheme()
{
    if (!m_statusBar)
        return;

    auto &theme = ThemeManager::instance();
    const QString text = theme.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString muted = theme.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
    const QString faint = theme.semanticColor(ThemeManager::SemanticRole::TextFaint).name();
    const QString accent = theme.semanticColor(ThemeManager::SemanticRole::Accent).name();

    applyLabelStyle(m_contextLabel, accent);
    applyLabelStyle(m_errorLabel, muted);
    applyLabelStyle(m_positionLabel, muted);
    applyLabelStyle(m_indentLabel, faint);
    applyLabelStyle(m_encodingLabel, faint);
    applyLabelStyle(m_languageLabel, accent, QStringLiteral("font-weight: bold;"));
    applyLabelStyle(m_vimLabel, text);

    const QString lspText = m_lspLabel ? m_lspLabel->text() : QString();
    QString lspColor = theme.semanticColor(ThemeManager::SemanticRole::Success).name();
    if (lspText.contains(QLatin1Char('!')))
        lspColor = theme.semanticColor(ThemeManager::SemanticRole::Error).name();
    else if (lspText.contains(QStringLiteral("○")) ||
             lspText.contains(QStringLiteral("◐")))
        lspColor = theme.semanticColor(ThemeManager::SemanticRole::Warning).name();
    else if (lspText.contains(QStringLiteral("Disabled")))
        lspColor = faint;

    setLspStatus(lspText, lspColor);
}

void StatusBarController::showMessage(const QString &message, int timeout)
{
    if (m_statusBar)
        m_statusBar->showMessage(message, timeout);
}

void StatusBarController::applyLabelStyle(QLabel *label, const QString &color,
                                          const QString &extra)
{
    if (label)
        label->setStyleSheet(paddedStyle(color, extra));
}
