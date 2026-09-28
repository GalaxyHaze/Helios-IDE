#ifndef LSPLOGPRESENTER_H
#define LSPLOGPRESENTER_H

#include <QObject>
#include <QString>

#include <functional>

class LspManagerDialog;
class SettingsPanel;

class LspLogPresenter : public QObject
{
public:
    using LspEnabled = std::function<bool()>;

    LspLogPresenter(SettingsPanel *settingsPanel,
                    LspManagerDialog *lspManagerDialog,
                    LspEnabled lspEnabled,
                    QObject *parent = nullptr);

    void append(const QString &message);

private:
    SettingsPanel *m_settingsPanel = nullptr;
    LspManagerDialog *m_lspManagerDialog = nullptr;
    LspEnabled m_lspEnabled;
};

#endif
