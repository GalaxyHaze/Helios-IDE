#ifndef LSPRUNTIMEPRESENTATIONSTATE_H
#define LSPRUNTIMEPRESENTATIONSTATE_H

#include <QString>

struct LspRuntimeInfo
{
    QString status;
    QString tag;
    QString lspPath;
    QString stdlibPath;
    QString cachePath;
};

struct ClangdInfo
{
    QString status;
    QString resolvedPath;
    QString message;
};

struct LspDiagnosticsInfo
{
    QString connection;
    QString syncMode;
    QString lastError;
};

#endif
