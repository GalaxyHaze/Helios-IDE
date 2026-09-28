#ifndef ZITHRUNTIMEOVERRIDERESOLVER_H
#define ZITHRUNTIMEOVERRIDERESOLVER_H

#include "ZithRuntimeCatalog.h"

#include <QString>

class ZithRuntimeOverrideResolver
{
public:
    enum class Action
    {
        NotConfigured,
        IgnoreAndContinue,
        Fail,
        Use
    };

    struct Result
    {
        Action action = Action::NotConfigured;
        ZithRuntimeCatalog::ResolvedRuntime runtime;
        QString message;
    };

    static Result resolve(const QString &lspPath, const QString &stdlibPath);
};

#endif
