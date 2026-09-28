#ifndef ZITHRUNTIMEASSETDOWNLOADER_H
#define ZITHRUNTIMEASSETDOWNLOADER_H

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

struct ZithRuntimeAssetDownload
{
    QUrl url;
    QString assetName;
    QString temporaryPath;
};

class ZithRuntimeAssetDownloader : public QObject
{
    Q_OBJECT

public:
    explicit ZithRuntimeAssetDownloader(QObject *parent = nullptr);

    void start(const ZithRuntimeAssetDownload &download);
    void cancel();
    bool isActive() const;

signals:
    void finished();
    void failed(const QString &message);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_reply = nullptr;
    ZithRuntimeAssetDownload m_currentDownload;
};

#endif
