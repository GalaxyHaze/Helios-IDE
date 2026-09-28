#ifndef ZITHRELEASECATALOG_H
#define ZITHRELEASECATALOG_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

#include <optional>

class ZithReleaseCatalog
{
public:
    struct Asset
    {
        QString name;
        QUrl downloadUrl;
    };

    struct Release
    {
        QString tag;
        QList<Asset> assets;
    };

    // Parses the GitHub latest-release payload without performing I/O.
    static std::optional<Release> parse(const QByteArray &payload,
                                         QString *errorMessage = nullptr);

    static std::optional<Asset> findLspAsset(const Release &release);
    static std::optional<Asset> findStdlibAsset(const Release &release);

private:
    static QString lspAssetNameForCurrentPlatform();
    static QString stdlibAssetSuffixForCurrentPlatform();
    static bool isArm64Architecture();
};

#endif
