#ifndef CLANGDEXECUTABLERESOLVER_H
#define CLANGDEXECUTABLERESOLVER_H

#include <QString>

class ClangdExecutableResolver
{
public:
    static QString resolve(const QString &configuredPath,
                           const QString &pathEnvironment);
};

#endif
