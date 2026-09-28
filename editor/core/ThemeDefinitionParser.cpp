#include "ThemeDefinitionParser.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <utility>

namespace
{
QColor parseColor(const QJsonObject &object, const QString &key,
                  const QString &fallback)
{
    return QColor(object.value(key).toString(fallback));
}

QPalette fallbackPalette(bool dark)
{
    QPalette palette;
    palette.setColor(QPalette::Window,
                     QColor(dark ? "#11131a" : "#e9edf2"));
    palette.setColor(QPalette::WindowText,
                     QColor(dark ? "#e6e9f2" : "#263043"));
    palette.setColor(QPalette::Base,
                     QColor(dark ? "#141720" : "#f3f5f7"));
    palette.setColor(QPalette::AlternateBase,
                     QColor(dark ? "#1d2230" : "#e2e7ed"));
    palette.setColor(QPalette::ToolTipBase,
                     QColor(dark ? "#252b3a" : "#f7f8fa"));
    palette.setColor(QPalette::ToolTipText,
                     QColor(dark ? "#edf0fa" : "#263043"));
    palette.setColor(QPalette::Text,
                     QColor(dark ? "#d9deeb" : "#2f3a4d"));
    palette.setColor(QPalette::Button,
                     QColor(dark ? "#202638" : "#dde4ec"));
    palette.setColor(QPalette::ButtonText,
                     QColor(dark ? "#e6e9f2" : "#283448"));
    palette.setColor(QPalette::BrightText,
                     QColor(dark ? "#ff7a90" : "#c4495f"));
    palette.setColor(QPalette::Link,
                     QColor(dark ? "#8fa2ff" : "#3f64bd"));
    palette.setColor(QPalette::Highlight,
                     QColor(dark ? "#3b5ccc" : "#3f6fa3"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    return palette;
}

QMap<QString, QColor> fallbackCustomColors(bool dark)
{
    if (dark) {
        return {
            {"sidebar", QColor("#0e1016")},
            {"sidebarBorder", QColor("#272d3d")},
            {"sidebarHover", QColor("#20283a")},
            {"sidebarActive", QColor("#1d2537")},
            {"sidebarActiveBorder", QColor("#8fa2ff")},
            {"tabWidgetPane", QColor("#141720")},
            {"tabBarBg", QColor("#11131a")},
            {"tabBg", QColor("#11131a")},
            {"tabFg", QColor("#8991a5")},
            {"tabBorder", QColor("#272d3d")},
            {"tabSelectedFg", QColor("#f1f3fb")},
            {"tabHoverBg", QColor("#1d2532")},
            {"treeBg", QColor("#141720")},
            {"treeFg", QColor("#d1d7e5")},
            {"treeHover", QColor("#27324a")},
            {"treeSelected", QColor("#32436a")},
            {"treeSelectedFg", QColor("#ffffff")},
            {"treeBranch", QColor("#141720")},
            {"editorBg", QColor("#1b1e2a")},
            {"editorFg", QColor("#e6e9f2")},
            {"editorLineNumber", QColor("#687188")},
            {"editorCurrentLine", QColor("#23293a")},
            {"editorSelection", QColor("#354b83")},
            {"gutterBg", QColor("#151821")},
            {"gutterActive", QColor("#aab8ff")},
            {"bracketBg", QColor("#41537c")},
            {"bracketFg", QColor("#ffffff")},
            {"diagnosticError", QColor("#ff7a90")},
            {"diagnosticWarning", QColor("#f1c77a")},
            {"diagnosticInfo", QColor("#70c9f0")},
            {"diagnosticUnknown", QColor("#aeb8ce")},
        };
    }

    return {
        {"sidebar", QColor("#e3e8ee")},
        {"sidebarBorder", QColor("#c4ccd7")},
        {"sidebarHover", QColor("#d5dde7")},
        {"sidebarActive", QColor("#cbd7e5")},
        {"sidebarActiveBorder", QColor("#4b6fae")},
        {"tabWidgetPane", QColor("#f3f5f7")},
        {"tabBarBg", QColor("#e9edf2")},
        {"tabBg", QColor("#e9edf2")},
        {"tabFg", QColor("#627087")},
        {"tabBorder", QColor("#c4ccd7")},
        {"tabSelectedFg", QColor("#263043")},
        {"tabHoverBg", QColor("#dce3eb")},
        {"treeBg", QColor("#e9edf2")},
        {"treeFg", QColor("#2f3a4d")},
        {"treeHover", QColor("#d4dee9")},
        {"treeSelected", QColor("#bfcfe2")},
        {"treeSelectedFg", QColor("#1e2a3d")},
        {"treeBranch", QColor("#e9edf2")},
        {"editorBg", QColor("#f8fafb")},
        {"editorFg", QColor("#263043")},
        {"editorLineNumber", QColor("#8793a5")},
        {"editorCurrentLine", QColor("#eaf0f5")},
        {"editorSelection", QColor("#b8cbe2")},
        {"gutterBg", QColor("#edf1f5")},
        {"gutterActive", QColor("#3f64a6")},
        {"bracketBg", QColor("#c8d7e9")},
        {"bracketFg", QColor("#1d2a40")},
        {"diagnosticError", QColor("#c4495f")},
        {"diagnosticWarning", QColor("#ab701d")},
        {"diagnosticInfo", QColor("#2e739b")},
        {"diagnosticUnknown", QColor("#637088")},
    };
}

QMap<QString, SyntaxStyle> fallbackSyntaxStyles()
{
    return {
        {"comment", {QColor("#6A5A8A"), false, true}},
        {"string", {QColor("#a6d189"), false, false}},
        {"number", {QColor("#04a5e5"), true, false}},
        {"type", {QColor("#ca9ee6"), true, false}},
        {"control", {QColor("#e64553"), true, false}},
        {"declaration", {QColor("#8839ef"), true, false}},
        {"storage", {QColor("#81c8be"), false, false}},
        {"async", {QColor("#a6d189"), true, false}},
        {"exception", {QColor("#df8e1d"), true, false}},
        {"keyword", {QColor("#eff1f5"), false, false}},
        {"jump", {QColor("#f38ba8"), true, false}},
        {"preprocessor", {QColor("#c9cbff"), false, false}},
        {"literal", {QColor("#dd7878"), true, false}},
        {"logicalOperator", {QColor("#d20f39"), true, false}},
        {"operator", {QColor("#7287fd"), false, false}},
        {"otherOperator", {QColor("#179299"), false, false}},
        {"bracket", {QColor("#ea999c"), false, false}},
        {"punctuation", {QColor("#a5adce"), false, false}},
    };
}
}

ThemeDefinition ThemeDefinitionParser::fallback(bool dark)
{
    return {fallbackPalette(dark), fallbackCustomColors(dark),
            fallbackSyntaxStyles(),};
}

std::optional<ThemeDefinition> ThemeDefinitionParser::parse(
    const QJsonDocument &document, bool fallbackDark)
{
    if (!document.isObject()) {
        return std::nullopt;
    }

    const QJsonObject object = document.object();
    const QJsonObject paletteObject = object.value("palette").toObject();
    const QPalette fallback = fallbackPalette(fallbackDark);
    QPalette palette = fallback;
    palette.setColor(
        QPalette::Window,
        parseColor(paletteObject, "window",
                   fallback.color(QPalette::Window).name()));
    palette.setColor(
        QPalette::WindowText,
        parseColor(paletteObject, "windowText",
                   fallback.color(QPalette::WindowText).name()));
    palette.setColor(
        QPalette::Base,
        parseColor(paletteObject, "base",
                   fallback.color(QPalette::Base).name()));
    palette.setColor(
        QPalette::AlternateBase,
        parseColor(paletteObject, "alternateBase",
                   fallback.color(QPalette::AlternateBase).name()));
    palette.setColor(
        QPalette::ToolTipBase,
        parseColor(paletteObject, "toolTipBase",
                   fallback.color(QPalette::ToolTipBase).name()));
    palette.setColor(
        QPalette::ToolTipText,
        parseColor(paletteObject, "toolTipText",
                   fallback.color(QPalette::ToolTipText).name()));
    palette.setColor(
        QPalette::Text,
        parseColor(paletteObject, "text",
                   fallback.color(QPalette::Text).name()));
    palette.setColor(
        QPalette::Button,
        parseColor(paletteObject, "button",
                   fallback.color(QPalette::Button).name()));
    palette.setColor(
        QPalette::ButtonText,
        parseColor(paletteObject, "buttonText",
                   fallback.color(QPalette::ButtonText).name()));
    palette.setColor(
        QPalette::BrightText,
        parseColor(paletteObject, "brightText",
                   fallback.color(QPalette::BrightText).name()));
    palette.setColor(
        QPalette::Link,
        parseColor(paletteObject, "link",
                   fallback.color(QPalette::Link).name()));
    palette.setColor(
        QPalette::Highlight,
        parseColor(paletteObject, "highlight",
                   fallback.color(QPalette::Highlight).name()));
    palette.setColor(
        QPalette::HighlightedText,
        parseColor(paletteObject, "highlightedText", "#ffffff"));

    ThemeDefinition definition{palette, {}, {}};
    QMap<QString, QColor> customColors;
    const QJsonObject customObject = object.value("custom").toObject();
    for (auto it = customObject.begin(); it != customObject.end(); ++it) {
        const QColor color(it.value().toString());
        if (color.isValid()) {
            customColors.insert(it.key(), color);
        }
    }

    QMap<QString, SyntaxStyle> syntaxStyles;
    const QJsonObject syntaxObject = object.value("syntax").toObject();
    for (auto it = syntaxObject.begin(); it != syntaxObject.end(); ++it) {
        const QJsonObject styleObject = it.value().toObject();
        const QColor color(styleObject.value("color").toString());
        if (color.isValid()) {
            syntaxStyles.insert(
                it.key(), {color, styleObject.value("bold").toBool(),
                           styleObject.value("italic").toBool(),});
        }
    }

    definition.customColors = std::move(customColors);
    definition.syntaxStyles = std::move(syntaxStyles);
    return definition;
}
