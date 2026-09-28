#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>

struct EditorSessionState
{
    QStringList openFiles;
    int currentTab = -1;
};

struct Context {
    QString rootPath;
    EditorSessionState session;
};

class ContextManager : public QObject
{
    Q_OBJECT

public:
    enum class ContextChangeReason
    {
        Navigation,
        RootChanged,
        NewContext
    };

    explicit ContextManager(QObject *parent = nullptr);

    int currentIndex() const { return m_current; }
    int count() const { return static_cast<int>(m_contexts.size()); }

    const Context &currentContext() const;
    QString currentRoot() const;

    void navigateLeft();
    void navigateRight();

    void setCurrentRoot(const QString &path);
    void appendNew(const QString &rootPath);
    void setContextState(int index, const EditorSessionState &session);

signals:
    void contextChanged(int index, const Context &context,
                        ContextChangeReason reason);

private:
    QList<Context> m_contexts;
    int m_current = 0;
};

#endif
