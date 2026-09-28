#include "ZithReleaseCatalog.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>

namespace
{
void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage != nullptr)
        *errorMessage = message;
}
}

std::optional<ZithReleaseCatalog::Release>
ZithReleaseCatalog::parse(const QByteArray &payload, QString *errorMessage)
{
    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject()) {
        setError(errorMessage, QStringLiteral("Latest Zith release response was not valid JSON."));
        return std::nullopt;
    }

    const QJsonObject root = document.object();
    Release release;
    release.tag = root.value(QStringLiteral("tag_name")).toString().trimmed();

    const QJsonArray assets = root.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue &assetValue : assets) {
        const QJsonObject assetObject = assetValue.toObject();
        const QString name = assetObject.value(QStringLiteral("name")).toString();
        const QString downloadUrl =
            assetObject.value(QStringLiteral("browser_download_url")).toString();
        if (name.isEmpty() || downloadUrl.isEmpty())
            continue;
        release.assets.append({name, QUrl(downloadUrl)});
    }

    if (release.tag.isEmpty()) {
        setError(errorMessage, QStringLiteral("Latest Zith release did not expose a tag."));
        return std::nullopt;
    }

    return release;
}

std::optional<ZithReleaseCatalog::Asset>
ZithReleaseCatalog::findLspAsset(const Release &release)
{
    const QString expectedName = lspAssetNameForCurrentPlatform();
    for (const Asset &asset : release.assets) {
        if (asset.name == expectedName)
            return asset;
    }
    return std::nullopt;
}

std::optional<ZithReleaseCatalog::Asset>
ZithReleaseCatalog::findStdlibAsset(const Release &release)
{
    const QString expectedSuffix = stdlibAssetSuffixForCurrentPlatform();
    for (const Asset &asset : release.assets) {
        if (asset.name.startsWith(QStringLiteral("zithc-stdlib-")) &&
            asset.name.endsWith(expectedSuffix)) {
            return asset;
        }
    }
    return std::nullopt;
}

QString ZithReleaseCatalog::lspAssetNameForCurrentPlatform()
{
#ifdef Q_OS_WIN
    return isArm64Architecture() ? QStringLiteral("zith-lsp-windows-arm64.exe")
                                 : QStringLiteral("zith-lsp-windows-amd64.exe");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("zith-lsp-macos-universal");
#else
    return isArm64Architecture() ? QStringLiteral("zith-lsp-linux-arm64")
                                 : QStringLiteral("zith-lsp-linux-amd64");
#endif
}

QString ZithReleaseCatalog::stdlibAssetSuffixForCurrentPlatform()
{
#ifdef Q_OS_WIN
    return QStringLiteral(".zip");
#else
    return QStringLiteral(".tar.gz");
#endif
}

bool ZithReleaseCatalog::isArm64Architecture()
{
    const QString arch = QSysInfo::currentCpuArchitecture().toLower();
    return arch.contains(QStringLiteral("arm64")) ||
        arch.contains(QStringLiteral("aarch64"));
}
