#ifndef EDITORLANGUAGEREQUESTCONTEXT_H
#define EDITORLANGUAGEREQUESTCONTEXT_H

#include "LspTypes.h"

#include <QString>

struct EditorLanguageRequestContext
{
    QString uri;
    int version = -1;
    LspPosition position;

    bool isValid() const
    {
        return !uri.isEmpty() && version >= 0 && position.line >= 0 &&
               position.character >= 0;
    }
};

#endif
