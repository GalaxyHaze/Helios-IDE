#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

class TranslationManager : public QObject
{
    Q_OBJECT
public:
    explicit TranslationManager(QObject *parent = nullptr);
    static TranslationManager &instance();

    bool loadLocale(const QString &locale);
    QString currentLocale() const { return m_currentLocale; }

    QString translate(const QString &key) const;

signals:
    void localeChanged();

private:
    QString m_currentLocale;
    QMap<QString, QString> m_translations;
    QMap<QString, QString> m_fallbackTranslations;
    bool m_currentLocaleWasLoaded = false;

    void loadFallbackTranslations();
    QString findI18nFile(const QString &locale);
};

#endif
