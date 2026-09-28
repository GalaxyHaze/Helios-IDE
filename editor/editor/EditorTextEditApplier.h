#ifndef EDITORTEXTEDITAPPLIER_H
#define EDITORTEXTEDITAPPLIER_H

#include "LspTypes.h"

#include <QList>
#include <QPair>

class QTextDocument;

class EditorTextEditApplier
{
public:
    using TextEdit = QPair<LspRange, QString>;

    static int offsetForLspPosition(const QTextDocument &document,
                                    const LspPosition &position);
    static void apply(QTextDocument &document, const QList<TextEdit> &edits);
};

#endif
