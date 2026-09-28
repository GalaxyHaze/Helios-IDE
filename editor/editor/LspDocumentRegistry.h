#ifndef LSPDOCUMENTREGISTRY_H
#define LSPDOCUMENTREGISTRY_H

#include <QHash>
#include <QString>

class LspDocumentRegistry
{
public:
    void clear();

    void open(const QString &uri, int version);
    void update(const QString &uri, int version);
    void close(const QString &uri);

    int version(const QString &uri) const;
    bool isCurrent(const QString &uri, int version) const;

private:
    QHash<QString, int> m_versions;
};

#endif
