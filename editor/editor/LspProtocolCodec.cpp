#include "LspProtocolCodec.h"

#include <QJsonDocument>
#include <QJsonParseError>

bool LspProtocolCodec::encode(const QJsonObject &message, QByteArray &frame,
                              QString &error)
{
    const QByteArray body =
        QJsonDocument(message).toJson(QJsonDocument::Compact);
    if (body.size() > MaxMessageSize) {
        error = QStringLiteral("Refused LSP message larger than 64 MB");
        frame.clear();
        return false;
    }

    frame = "Content-Length: " + QByteArray::number(body.size()) +
            "\r\n\r\n" + body;
    error.clear();
    return true;
}

LspProtocolCodec::DecodeResult
LspProtocolCodec::consume(const QByteArray &chunk)
{
    DecodeResult result;
    m_buffer.append(chunk);
    if (m_buffer.size() > MaxMessageSize + 8192) {
        m_buffer.clear();
        result.inputTooLarge = true;
        return result;
    }

    while (true) {
        const qsizetype headerEnd = m_buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0)
            break;

        if (headerEnd > 8192) {
            m_buffer.remove(0, headerEnd + 4);
            result.errors.append(
                QStringLiteral("Discarded oversized LSP header"));
            continue;
        }

        bool foundLength = false;
        bool validLength = false;
        qint64 length = 0;
        for (const QByteArray &line :
             m_buffer.left(headerEnd).split('\n')) {
            const QByteArray trimmed = line.trimmed();
            const qsizetype colon = trimmed.indexOf(':');
            if (colon > 0 &&
                trimmed.left(colon).compare("Content-Length",
                                             Qt::CaseInsensitive) == 0) {
                foundLength = true;
                length =
                    trimmed.mid(colon + 1).trimmed().toLongLong(&validLength);
                break;
            }
        }

        if (!foundLength || !validLength || length < 0 ||
            length > MaxMessageSize) {
            m_buffer.remove(0, headerEnd + 4);
            result.errors.append(
                QStringLiteral("Discarded malformed LSP frame header"));
            continue;
        }

        const qint64 bodyStart = headerEnd + 4;
        if (m_buffer.size() < bodyStart + length)
            break;

        const QByteArray body = m_buffer.mid(bodyStart, length);
        m_buffer.remove(0, bodyStart + length);

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(body, &parseError);
        if (parseError.error == QJsonParseError::NoError &&
            document.isObject()) {
            result.messages.append(document.object());
        }
    }

    return result;
}

void LspProtocolCodec::reset()
{
    m_buffer.clear();
}
