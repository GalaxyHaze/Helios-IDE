#ifndef LSPRUNTIMEPRESENTATIONMODEL_H
#define LSPRUNTIMEPRESENTATIONMODEL_H

#include "LspRuntimePresentationState.h"

#include <functional>

class LspRuntimePresentationModel
{
public:
    using Translate = std::function<QString(const QString &)>;

    explicit LspRuntimePresentationModel(Translate translate = {});

    void setRuntimeInfo(const LspRuntimeInfo &info);
    void setDiagnostics(const LspDiagnosticsInfo &info);
    void setClangdInfo(const ClangdInfo &info);

    LspRuntimeInfo displayRuntimeInfo() const;
    LspDiagnosticsInfo displayDiagnostics() const;
    ClangdInfo displayClangdInfo() const;

private:
    QString translate(const QString &key) const;

    LspRuntimeInfo m_runtimeInfo;
    LspDiagnosticsInfo m_diagnostics;
    ClangdInfo m_clangdInfo;
    Translate m_translate;
};

#endif
