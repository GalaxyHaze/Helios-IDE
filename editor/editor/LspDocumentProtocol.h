#ifndef LSPDOCUMENTPROTOCOL_H
#define LSPDOCUMENTPROTOCOL_H

#include "LspDocumentRegistry.h"
#include "LspTypes.h"

#include <QJsonObject>
#include <QList>
#include <QString>

#include <functional>

struct LspDocumentProtocolCallbacks
{
    std::function<bool()> isReady;
    std::function<bool(const QJsonObject &)> sendMessage;
    std::function<void(const QString &)> cancelRequestsForUri;
};

class LspDocumentProtocol
{
public:
    explicit LspDocumentProtocol(LspDocumentProtocolCallbacks callbacks);

    void clear();

    int documentVersion(const QString &uri) const;
    bool isCurrentDocument(const QString &uri, int version) const;

    void openDocument(const QString &uri, const QString &languageId,
                      const QString &text, int version = 1);
    void changeDocument(const QString &uri,
                        const QList<LspTextChange> &changes,
                        int version);
    void changeDocumentFull(const QString &uri, const QString &fullText,
                            int version);
    void closeDocument(const QString &uri);
    void saveDocument(const QString &uri);

private:
    LspDocumentProtocolCallbacks m_callbacks;
    LspDocumentRegistry m_documentRegistry;
};

#endif
