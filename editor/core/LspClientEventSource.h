#ifndef LSPCLIENTEVENTSOURCE_H
#define LSPCLIENTEVENTSOURCE_H

#include "LspEventSource.h"

class LspClient;

class LspClientEventSource : public LspEventSource
{
    Q_OBJECT

public:
    explicit LspClientEventSource(LspClient *client,
                                  QObject *parent = nullptr);

private:
    LspClient *m_client = nullptr;
};

#endif
