#ifndef LSPPROTOCOLCODEC_H
#define LSPPROTOCOLCODEC_H

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QStringList>

class LspProtocolCodec
{
public:
    static constexpr qsizetype MaxMessageSize = 64LL * 1024 * 1024;

    struct DecodeResult
    {
        QList<QJsonObject> messages;
        QStringList errors;
        bool inputTooLarge = false;
    };

    static bool encode(const QJsonObject &message, QByteArray &frame,
                       QString &error);

    DecodeResult consume(const QByteArray &chunk);
    void reset();

private:
    QByteArray m_buffer;
};

#endif
