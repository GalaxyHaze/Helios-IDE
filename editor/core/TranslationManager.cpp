#include "TranslationManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
{
    loadFallbackTranslations();
}

TranslationManager &TranslationManager::instance()
{
    static TranslationManager manager;
    return manager;
}

QString TranslationManager::findI18nFile(const QString &locale)
{
    QString appDir = QCoreApplication::applicationDirPath() + "/i18n/";
    QString path = appDir + locale + ".json";
    if (QFileInfo::exists(path)) return path;

    path = "/home/diogo/Helios/i18n/" + locale + ".json";
    if (QFileInfo::exists(path)) return path;

    return "";
}

void TranslationManager::loadFallbackTranslations()
{
    m_fallbackTranslations.clear();
    QString path = findI18nFile("en-US");
    QFile file(path);
    if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            m_fallbackTranslations.insert(it.key(), it.value().toString());
        }
    }
}

bool TranslationManager::loadLocale(const QString &locale)
{
    if (locale == m_currentLocale)
        return m_currentLocaleWasLoaded;

    m_currentLocale = locale;
    m_translations.clear();

    QString path = findI18nFile(locale);
    QFile file(path);
    if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        m_currentLocaleWasLoaded = false;
        emit localeChanged();
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        m_currentLocaleWasLoaded = false;
        emit localeChanged();
        return false;
    }

    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_translations.insert(it.key(), it.value().toString());
    }

    m_currentLocaleWasLoaded = true;
    emit localeChanged();
    return true;
}

QString TranslationManager::translate(const QString &key) const
{
    if (m_translations.contains(key)) {
        return m_translations.value(key);
    }
    if (m_fallbackTranslations.contains(key)) {
        return m_fallbackTranslations.value(key);
    }
    return key;
}
