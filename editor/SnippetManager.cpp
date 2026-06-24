#include "SnippetManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

SnippetManager::SnippetManager(QObject *parent)
    : QObject(parent)
{
}

bool SnippetManager::loadFromJson(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return false;

    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QJsonObject snippet = it.value().toObject();
        Snippet s;
        s.prefix = snippet["prefix"].toString();
        s.description = snippet["description"].toString();

        QJsonArray bodyArr;
        if (snippet["body"].isString()) {
            bodyArr.append(snippet["body"].toString());
        } else {
            bodyArr = snippet["body"].toArray();
        }

        QStringList lines;
        for (const auto &v : bodyArr)
            lines.append(v.toString());
        s.body = processBody(lines.join("\n"));

        m_snippets.insert(s.prefix, s);
    }
    return true;
}

QString SnippetManager::expand(const QString &prefix) const
{
    auto it = m_snippets.find(prefix);
    if (it == m_snippets.end())
        return {};
    return it->body;
}

bool SnippetManager::hasSnippet(const QString &prefix) const
{
    return m_snippets.contains(prefix);
}

QString SnippetManager::processBody(const QString &raw)
{
    static QRegularExpression re(R"(\$\{?\d+(?::([^}]*))?\}?)");
    QString result;
    int lastEnd = 0;
    auto it = re.globalMatch(raw);
    while (it.hasNext()) {
        auto match = it.next();
        result += raw.mid(lastEnd, match.capturedStart() - lastEnd);
        QString defaultVal = match.captured(1);
        if (!defaultVal.isEmpty())
            result += defaultVal;
        lastEnd = match.capturedEnd();
    }
    result += raw.mid(lastEnd);
    result.replace("$$", "$");
    result.replace(QChar('\t'), "    ");
    return result;
}
