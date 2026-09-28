#ifndef ZITHRUNTIMESTATE_H
#define ZITHRUNTIMESTATE_H

#include <QString>

class ZithRuntimeState
{
public:
    void setStatusText(const QString &text) { m_statusText = text; }
    void activate(const QString &lspPath, const QString &stdlibPath,
                  const QString &tag, const QString &workspaceRoot);
    void clearRuntime();

    bool matches(const QString &lspPath, const QString &stdlibPath,
                 const QString &workspaceRoot) const;

    const QString &statusText() const { return m_statusText; }
    const QString &tag() const { return m_tag; }
    const QString &lspPath() const { return m_lspPath; }
    const QString &stdlibPath() const { return m_stdlibPath; }
    const QString &workspaceRoot() const { return m_workspaceRoot; }

private:
    QString m_statusText;
    QString m_tag;
    QString m_lspPath;
    QString m_stdlibPath;
    QString m_workspaceRoot;
};

#endif
