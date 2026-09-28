#include "LspLogPresenter.h"

#include "../panels/LspManagerDialog.h"
#include "../panels/SettingsPanel.h"

#include <utility>

LspLogPresenter::LspLogPresenter(SettingsPanel *settingsPanel,
                                 LspManagerDialog *lspManagerDialog,
                                 LspEnabled lspEnabled, QObject *parent)
    : QObject(parent),
      m_settingsPanel(settingsPanel),
      m_lspManagerDialog(lspManagerDialog),
      m_lspEnabled(std::move(lspEnabled))
{
}

void LspLogPresenter::append(const QString &message)
{
    if ((!m_lspEnabled || m_lspEnabled()) && m_settingsPanel)
        m_settingsPanel->appendLspLog(message);
    if (m_lspManagerDialog)
        m_lspManagerDialog->appendLspLog(message);
}
