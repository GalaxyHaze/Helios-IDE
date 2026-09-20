#ifndef DIAGNOSTICSPANEL_H
#define DIAGNOSTICSPANEL_H

#include <QListWidget>
#include <QMap>
#include <QSet>
#include <QWidget>
#include "../editor/LspClient.h"

class CodeEditor;
class DiagnosticsPanel;

class DiagnosticsPanelItem {
public:
    QString uri;
    LspDiagnostic diagnostic;
};

class DiagnosticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DiagnosticsPanel(QWidget *parent = nullptr);

    void clear();
    void setDiagnostics(const QString &uri, int version, const QList<LspDiagnostic> &diagnostics);
    void clearDiagnostics(const QString &uri);
    QList<LspDiagnostic> allDiagnostics() const;

    int errorCount() const { return m_errorCount; }
    int warningCount() const { return m_warningCount; }

signals:
    void navigateToLocation(const QString &uri, int line, int character);
    void countsChanged(int errors, int warnings);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void updateTheme();

private:
    QListWidget *m_list;
    QMap<QString, QList<LspDiagnostic>> m_diagnosticsByUri;
    void rebuild();
    int m_errorCount = 0;
    int m_warningCount = 0;
};

#endif // DIAGNOSTICSPANEL_H
