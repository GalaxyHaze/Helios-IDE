#include "LanguageIdentity.h"

#include <array>

#include <QFileInfo>

LanguageIdentity::Language LanguageIdentity::forPath(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QLatin1String("zith"))
        return Language::Zith;

    static constexpr std::array<const char *, 8> cFamilySuffixes = {
        "c", "h", "cc", "cpp", "cxx", "hh", "hpp", "hxx"};
    for (const char *candidate : cFamilySuffixes) {
        if (suffix == QLatin1String(candidate))
            return Language::CFamily;
    }

    return Language::PlainText;
}

QString LanguageIdentity::lspId(Language language)
{
    switch (language) {
    case Language::Zith:
        return QStringLiteral("zith");
    case Language::CFamily:
        return QStringLiteral("cpp");
    case Language::PlainText:
        return QString();
    }

    return QString();
}
