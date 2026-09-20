#ifndef REFERENCESPANEL_H
#define REFERENCESPANEL_H

#include <QListWidget>
#include <QWidget>
#include "../editor/LspClient.h"

class ReferencesPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ReferencesPanel(QWidget *parent = nullptr);
    void setReferences(const QList<LspLocation> &references);
    void clearReferences();
    void applyTheme();

signals:
    void navigateToLocation(const QString &uri, int line, int character);

private:
    QListWidget *m_list = nullptr;
};

#endif
