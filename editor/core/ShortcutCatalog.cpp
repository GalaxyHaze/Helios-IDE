#include "ShortcutCatalog.h"

QList<ShortcutCategory> ShortcutCatalog::categories()
{
    return {
        {QStringLiteral("shortcut.cat.file"),
         {
             {QStringLiteral("shortcut.new_file"), QStringLiteral("Ctrl+N")},
             {QStringLiteral("shortcut.new_window"),
              QStringLiteral("Ctrl+Shift+N")},
             {QStringLiteral("shortcut.new_project"),
              QStringLiteral("Ctrl+Alt+N")},
             {QStringLiteral("shortcut.open"), QStringLiteral("Ctrl+O")},
             {QStringLiteral("shortcut.open_folder"),
              QStringLiteral("Ctrl+Alt+O")},
             {QStringLiteral("shortcut.save"), QStringLiteral("Ctrl+S")},
             {QStringLiteral("shortcut.exit"), QStringLiteral("Ctrl+Q")}
         }},
        {QStringLiteral("shortcut.cat.edit_search"),
         {
             {QStringLiteral("shortcut.find"), QStringLiteral("Ctrl+F")},
             {QStringLiteral("shortcut.replace"), QStringLiteral("Ctrl+H")},
             {QStringLiteral("shortcut.find_next"),
              QStringLiteral("F3 / Ctrl+G")},
             {QStringLiteral("shortcut.find_prev"),
              QStringLiteral("Shift+F3 / Ctrl+Shift+G")}
         }},
        {QStringLiteral("shortcut.cat.navigation_view"),
         {
             {QStringLiteral("shortcut.explorer"),
              QStringLiteral("Ctrl+Shift+E")},
             {QStringLiteral("shortcut.global_search"),
              QStringLiteral("Ctrl+Shift+F")},
             {QStringLiteral("shortcut.git_panel"),
              QStringLiteral("Ctrl+Shift+G")},
             {QStringLiteral("shortcut.settings"), QStringLiteral("Ctrl+,")},
             {QStringLiteral("shortcut.hide_sidebar"),
              QStringLiteral("Ctrl+Shift+X")},
             {QStringLiteral("shortcut.go_back"), QStringLiteral("Alt+Left")},
             {QStringLiteral("shortcut.go_forward"),
              QStringLiteral("Alt+Right")}
         }},
        {QStringLiteral("shortcut.cat.lsp_tools"),
         {
             {QStringLiteral("shortcut.go_definition"),
              QStringLiteral("F12 / Ctrl+Click")},
             {QStringLiteral("shortcut.go_implementation"),
              QStringLiteral("Ctrl+F12")},
             {QStringLiteral("shortcut.format_doc"),
              QStringLiteral("Ctrl+Alt+L / Alt+Shift+F")},
             {QStringLiteral("shortcut.completion"),
              QStringLiteral("Ctrl+Space")},
             {QStringLiteral("shortcut.build"), QStringLiteral("Ctrl+B")},
             {QStringLiteral("shortcut.check"),
              QStringLiteral("Ctrl+Shift+C")},
             {QStringLiteral("shortcut.run"),
              QStringLiteral("Ctrl+Shift+R")},
             {QStringLiteral("shortcut.stop"),
              QStringLiteral("Ctrl+Shift+Q")},
             {QStringLiteral("shortcut.restart_lsp"),
              QStringLiteral("Ctrl+Shift+L")},
             {QStringLiteral("shortcut.getting_started"),
              QStringLiteral("F1")}
         }}
    };
}
