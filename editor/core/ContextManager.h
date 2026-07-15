#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>

struct Context {
    QString rootPath;
    QStringList openFiles;
    int currentTab = -1;
};

class ContextManager : public QObject
{
    Q_OBJECT

public:
    explicit ContextManager(QObject *parent = nullptr);

    int currentIndex() const { return m_current; }
    int count() const { return m_contexts.size(); }

    const Context &currentContext() const;
    QString currentRoot() const;

    void navigateLeft();
    void navigateRight();

    void setCurrentRoot(const QString &path);
    void appendNew(const QString &rootPath);
    void setContextState(int index, const QStringList &files, int currentTab);

signals:
    void contextChanged(int index, const Context &context);

private:
    QList<Context> m_contexts;
    int m_current = 0;
};

#endif
