#ifndef BOTTOMPANEL_H
#define BOTTOMPANEL_H

#include <QDockWidget>

class QTabBar;
class QStackedWidget;
class QToolButton;
class DiagnosticsPanel;
class CompilerPanel;
class ReferencesPanel;

class BottomPanel : public QDockWidget
{
    Q_OBJECT

public:
    enum class Tab
    {
        Diagnostics = 0,
        Compiler = 1,
        References = 2
    };

    explicit BottomPanel(QWidget *parent = nullptr);

    DiagnosticsPanel *diagnostics() const { return m_diagnostics; }
    CompilerPanel *compiler() const { return m_compiler; }
    ReferencesPanel *references() const { return m_references; }

    void showTab(Tab tab);
    void showDiagnostics() { showTab(Tab::Diagnostics); }
    void showCompiler() { showTab(Tab::Compiler); }
    void showReferences() { showTab(Tab::References); }
    void clearCurrent();
    void setDiagnosticsCount(int errors, int warnings);
    void applyTheme();

signals:
    void closeRequested();

private slots:
    void onCloseClicked();

private:
    QTabBar *m_tabBar = nullptr;
    QStackedWidget *m_stack = nullptr;
    QToolButton *m_clearButton = nullptr;
    QToolButton *m_closeButton = nullptr;
    DiagnosticsPanel *m_diagnostics = nullptr;
    CompilerPanel *m_compiler = nullptr;
    ReferencesPanel *m_references = nullptr;
    int m_errorCount = 0;
    int m_warningCount = 0;
};

#endif // BOTTOMPANEL_H
