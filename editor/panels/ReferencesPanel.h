#ifndef REFERENCESPANEL_H
#define REFERENCESPANEL_H

#include <QDockWidget>
#include <QListWidget>
#include "../editor/LspClient.h"

class ReferencesPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit ReferencesPanel(QWidget *parent = nullptr);
    void setReferences(const QList<LspLocation> &references);
    void clearReferences();

signals:
    void navigateToLocation(const QString &uri, int line, int character);

private:
    QListWidget *m_list = nullptr;
};

#endif
