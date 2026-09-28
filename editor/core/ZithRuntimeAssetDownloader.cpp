#include "ZithRuntimeAssetDownloader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>

namespace
{
constexpr int kNetworkTimeoutMs = 5000;
}

ZithRuntimeAssetDownloader::ZithRuntimeAssetDownloader(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

void ZithRuntimeAssetDownloader::start(
    const ZithRuntimeAssetDownload &download)
{
    cancel();
    m_currentDownload = download;

    if (!m_currentDownload.url.isValid() ||
        m_currentDownload.assetName.isEmpty() ||
        m_currentDownload.temporaryPath.isEmpty()) {
        emit failed(QStringLiteral(
            "Zith runtime asset download request was incomplete."));
        return;
    }

    const QFileInfo destinationInfo(m_currentDownload.temporaryPath);
    if (!QDir().mkpath(destinationInfo.absolutePath())) {
        emit failed(QString("Failed to create the download directory for %1.")
                        .arg(m_currentDownload.assetName));
        return;
    }

    QNetworkRequest request(m_currentDownload.url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Helios"));
    request.setTransferTimeout(kNetworkTimeoutMs);

    m_reply = m_networkManager->get(request);
    connect(m_reply, &QNetworkReply::finished, this,
            &ZithRuntimeAssetDownloader::onReplyFinished);
}

void ZithRuntimeAssetDownloader::cancel()
{
    if (!m_reply)
        return;

    QObject::disconnect(m_reply, nullptr, this, nullptr);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
}

bool ZithRuntimeAssetDownloader::isActive() const
{
    return m_reply != nullptr;
}

void ZithRuntimeAssetDownloader::onReplyFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;

    if (!reply)
        return;

    const QByteArray payload = reply->readAll();
    const QString errorString =
        reply->error() == QNetworkReply::NoError ? QString()
                                                  : reply->errorString();
    reply->deleteLater();

    if (!errorString.isEmpty()) {
        QFile::remove(m_currentDownload.temporaryPath);
        emit failed("Failed to download " + m_currentDownload.assetName +
                    ": " + errorString);
        return;
    }

    QSaveFile file(m_currentDownload.temporaryPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit failed("Failed to write " + m_currentDownload.assetName +
                    " to cache.");
        return;
    }

    if (file.write(payload) != payload.size() || !file.commit()) {
        QFile::remove(m_currentDownload.temporaryPath);
        emit failed("Incomplete write while caching " +
                    m_currentDownload.assetName + ".");
        return;
    }

    emit finished();
}
