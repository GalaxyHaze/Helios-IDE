#ifndef LSPSETTINGSPERSISTENCE_H
#define LSPSETTINGSPERSISTENCE_H

#include <QString>

class TomlSettingsStore;

class LspSettingsPersistence
{
public:
    virtual ~LspSettingsPersistence() = default;

    virtual bool lspEnabled() const = 0;
    virtual bool useOnlineZithLsp() const = 0;
    virtual bool cLspEnabled() const = 0;
    virtual QString cLspPath() const = 0;

    virtual void setLspEnabled(bool enabled) = 0;
    virtual void setUseOnlineZithLsp(bool enabled) = 0;
    virtual void setCLspEnabled(bool enabled) = 0;
    virtual void setCLspPath(const QString &path) = 0;
};

class TomlLspSettingsPersistence final : public LspSettingsPersistence
{
public:
    explicit TomlLspSettingsPersistence(TomlSettingsStore &store);

    bool lspEnabled() const override;
    bool useOnlineZithLsp() const override;
    bool cLspEnabled() const override;
    QString cLspPath() const override;

    void setLspEnabled(bool enabled) override;
    void setUseOnlineZithLsp(bool enabled) override;
    void setCLspEnabled(bool enabled) override;
    void setCLspPath(const QString &path) override;

private:
    TomlSettingsStore &m_store;
};

#endif
