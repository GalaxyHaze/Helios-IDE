#include "DiagnosticsPanel.h"
#include <QVBoxLayout>
#include <QFont>
#include <QUrl>
#include <QFileInfo>

DiagnosticsPanel::DiagnosticsPanel(QWidget *parent)
    : QDockWidget("Diagnostics", parent)
{
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

    m_list = new QListWidget;
    m_list->setFont(QFont("monospace", 10));
    m_list->setWordWrap(true);
    m_list->setStyleSheet(
        "QListWidget { background: #11111b; color: #c6d0f5; border: none; }"
        "QListWidget::item { padding: 2px 4px; border-bottom: 1px solid #1e1e2e; }"
        "QListWidget::item:selected { background: #363a4f; }"
        "QListWidget::item:alternate { background: #181825; }"
    );

    setWidget(m_list);

    connect(m_list, &QListWidget::itemClicked, this, &DiagnosticsPanel::onItemClicked);
}

void DiagnosticsPanel::clear()
{
    m_list->clear();
    m_errorCount = 0;
    m_warningCount = 0;
    emit countsChanged(0, 0);
}

void DiagnosticsPanel::setDiagnostics(const QString &uri, const QList<LspDiagnostic> &diagnostics)
{
    m_list->clear();
    m_errorCount = 0;
    m_warningCount = 0;

    for (const LspDiagnostic &d : diagnostics) {
        QString severity;
        QColor color;
        switch (d.severity) {
        case 1: severity = "E"; color = QColor("#e64553"); m_errorCount++; break;
        case 2: severity = "W"; color = QColor("#df8e1d"); m_warningCount++; break;
        case 3: severity = "I"; color = QColor("#04a5e5"); break;
        default: severity = "?"; color = QColor("#a5adce"); break;
        }

        QFileInfo fi(QUrl(uri).toLocalFile());
        QString text = QString("[%1] %2:%3:%4 - %5")
            .arg(severity,
                 fi.fileName(),
                 QString::number(d.range.start.line + 1),
                 QString::number(d.range.start.character + 1),
                 d.message);

        auto *item = new QListWidgetItem(text);
        item->setForeground(color);
        item->setData(Qt::UserRole, uri);
        item->setData(Qt::UserRole + 1, d.range.start.line);
        item->setData(Qt::UserRole + 2, d.range.start.character);
        m_list->addItem(item);
    }

    emit countsChanged(m_errorCount, m_warningCount);
}

void DiagnosticsPanel::onItemClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    QString uri = item->data(Qt::UserRole).toString();
    int line = item->data(Qt::UserRole + 1).toInt();
    int character = item->data(Qt::UserRole + 2).toInt();

    emit navigateToLocation(uri, line, character);
}
