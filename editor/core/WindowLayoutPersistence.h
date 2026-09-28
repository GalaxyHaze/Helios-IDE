#ifndef WINDOWLAYOUTPERSISTENCE_H
#define WINDOWLAYOUTPERSISTENCE_H

#include <QByteArray>

class TomlSettingsStore;

class WindowLayoutPersistence
{
public:
    virtual ~WindowLayoutPersistence() = default;

    virtual QByteArray mainWindowGeometry() const = 0;
    virtual QByteArray mainWindowState() const = 0;
    virtual int sidebarWidth() const = 0;
    virtual bool sidebarVisible() const = 0;
    virtual bool outlineVisible() const = 0;

    virtual void setMainWindowGeometry(const QByteArray &geometry) = 0;
    virtual void setMainWindowState(const QByteArray &state) = 0;
    virtual void setSidebarWidth(int width) = 0;
    virtual void setSidebarVisible(bool visible) = 0;
    virtual void setOutlineVisible(bool visible) = 0;
};

class TomlWindowLayoutPersistence final : public WindowLayoutPersistence
{
public:
    explicit TomlWindowLayoutPersistence(TomlSettingsStore &store);

    QByteArray mainWindowGeometry() const override;
    QByteArray mainWindowState() const override;
    int sidebarWidth() const override;
    bool sidebarVisible() const override;
    bool outlineVisible() const override;

    void setMainWindowGeometry(const QByteArray &geometry) override;
    void setMainWindowState(const QByteArray &state) override;
    void setSidebarWidth(int width) override;
    void setSidebarVisible(bool visible) override;
    void setOutlineVisible(bool visible) override;

private:
    TomlSettingsStore &m_store;
};

#endif
