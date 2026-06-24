#ifndef SNIPPETMANAGER_H
#define SNIPPETMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QList>
#include "LspClient.h"

class SnippetManager : public QObject
{
    Q_OBJECT

public:
    explicit SnippetManager(QObject *parent = nullptr);

    bool loadFromJson(const QString &jsonPath);
    QString expand(const QString &prefix) const;
    bool hasSnippet(const QString &prefix) const;
    QList<LspCompletionItem> allSnippets() const;

private:
    struct Snippet {
        QString prefix;
        QString body;
        QString description;
        QString rawBody;
    };
    QHash<QString, Snippet> m_snippets;

    static QString processBody(const QString &raw);
};

#endif
