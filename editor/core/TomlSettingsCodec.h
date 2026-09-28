#ifndef TOMLSETTINGSCODEC_H
#define TOMLSETTINGSCODEC_H

#include "TomlSettingsSnapshot.h"

class TomlSettingsCodec
{
public:
    static TomlSettingsSnapshot parse(
        const QString &contents,
        const TomlSettingsSnapshot &defaults = TomlSettingsSnapshot{});
    static QString serialize(const TomlSettingsSnapshot &settings);
};

#endif
