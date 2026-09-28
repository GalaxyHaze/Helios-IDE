#ifndef THEMEDEFINITION_H
#define THEMEDEFINITION_H

#include <QColor>
#include <QMap>
#include <QPalette>

struct SyntaxStyle
{
    QColor color;
    bool bold = false;
    bool italic = false;
};

struct ThemeDefinition
{
    QPalette palette;
    QMap<QString, QColor> customColors;
    QMap<QString, SyntaxStyle> syntaxStyles;
};

#endif
