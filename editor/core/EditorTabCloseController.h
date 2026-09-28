#ifndef EDITORTABCLOSECONTROLLER_H
#define EDITORTABCLOSECONTROLLER_H

#include <QObject>
#include <functional>

class QTabWidget;
class CodeEditor;

class EditorTabCloseController : public QObject
{
    Q_OBJECT

public:
    enum class SaveDecision {
        Save,
        Discard,
        Cancel
    };

    struct Callbacks {
        std::function<SaveDecision(CodeEditor *)> requestSaveDecision;
        std::function<bool(CodeEditor *)> saveEditor;
        std::function<void(CodeEditor *)> releaseEditor;
    };

    explicit EditorTabCloseController(QTabWidget *tabWidget,
                                      Callbacks callbacks,
                                      QObject *parent = nullptr);

private:
    void closeTab(int index);

    QTabWidget *m_tabWidget = nullptr;
    Callbacks m_callbacks;
};

#endif
