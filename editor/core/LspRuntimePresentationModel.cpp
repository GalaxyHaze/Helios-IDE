#include "LspRuntimePresentationModel.h"

#include "TranslationManager.h"

#include <utility>

LspRuntimePresentationModel::LspRuntimePresentationModel(Translate translate)
    : m_translate(std::move(translate))
{
    if (!m_translate) {
        m_translate = [](const QString &key) {
            return TranslationManager::instance().translate(key);
        };
    }
}

QString LspRuntimePresentationModel::translate(const QString &key) const
{
    return m_translate(key);
}

void LspRuntimePresentationModel::setRuntimeInfo(
    const LspRuntimeInfo &info)
{
    m_runtimeInfo = info;
}

void LspRuntimePresentationModel::setDiagnostics(
    const LspDiagnosticsInfo &info)
{
    m_diagnostics = info;
}

void LspRuntimePresentationModel::setClangdInfo(const ClangdInfo &info)
{
    m_clangdInfo = info;
}

LspRuntimeInfo LspRuntimePresentationModel::displayRuntimeInfo() const
{
    LspRuntimeInfo display = m_runtimeInfo;
    display.status =
        display.status.isEmpty() ? translate("lsp.val_unavailable")
                                 : display.status;
    display.tag =
        display.tag.isEmpty() ? translate("lsp.val_unavailable") : display.tag;
    display.lspPath =
        display.lspPath.isEmpty() ? translate("lsp.val_unavailable")
                                  : display.lspPath;
    display.stdlibPath =
        display.stdlibPath.isEmpty() ? translate("lsp.val_unavailable")
                                     : display.stdlibPath;
    display.cachePath =
        display.cachePath.isEmpty() ? translate("lsp.val_unavailable")
                                    : display.cachePath;
    return display;
}

LspDiagnosticsInfo LspRuntimePresentationModel::displayDiagnostics() const
{
    LspDiagnosticsInfo display = m_diagnostics;
    display.connection =
        display.connection.isEmpty() ? translate("lsp.val_unknown")
                                     : display.connection;
    display.syncMode =
        display.syncMode.isEmpty() ? translate("lsp.val_unknown")
                                   : display.syncMode;
    display.lastError =
        display.lastError.isEmpty() ? translate("lsp.val_none")
                                    : display.lastError;
    return display;
}

ClangdInfo LspRuntimePresentationModel::displayClangdInfo() const
{
    ClangdInfo display = m_clangdInfo;
    display.status =
        display.status.isEmpty() ? translate("lsp.val_unavailable")
                                 : display.status;
    display.resolvedPath =
        display.resolvedPath.isEmpty() ? translate("lsp.val_unavailable")
                                       : display.resolvedPath;
    return display;
}
