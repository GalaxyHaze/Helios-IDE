#ifndef THEMEDEFINITIONPARSER_H
#define THEMEDEFINITIONPARSER_H

#include "ThemeDefinition.h"

#include <optional>

class QJsonDocument;

class ThemeDefinitionParser
{
public:
    static ThemeDefinition fallback(bool dark);
    static std::optional<ThemeDefinition> parse(
        const QJsonDocument &document, bool fallbackDark);
};

#endif
