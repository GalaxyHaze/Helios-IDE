#ifndef LANGUAGEIDENTITY_H
#define LANGUAGEIDENTITY_H

#include <QString>

class LanguageIdentity
{
public:
    enum class Language
    {
        Zith,
        CFamily,
        PlainText
    };

    static Language forPath(const QString &path);
    static QString lspId(Language language);
};

#endif
