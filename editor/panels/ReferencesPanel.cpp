#include "ReferencesPanel.h"

#include <QFileInfo>
#include <QUrl>

ReferencesPanel::ReferencesPanel(QWidget *parent)
    : QDockWidget("References", parent) {
  setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
  m_list = new QListWidget(this);
  setWidget(m_list);
  connect(m_list, &QListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
            emit navigateToLocation(item->data(Qt::UserRole).toString(),
                                    item->data(Qt::UserRole + 1).toInt(),
                                    item->data(Qt::UserRole + 2).toInt());
          });
}

void ReferencesPanel::clearReferences() { m_list->clear(); }

void ReferencesPanel::setReferences(const QList<LspLocation> &references) {
  clearReferences();
  for (const LspLocation &location : references) {
    const QString path = QUrl(location.uri).toLocalFile();
    auto *item =
        new QListWidgetItem(QString("%1:%2:%3")
                                .arg(QFileInfo(path).fileName())
                                .arg(location.range.start.line + 1)
                                .arg(location.range.start.character + 1),
                            m_list);
    item->setData(Qt::UserRole, location.uri);
    item->setData(Qt::UserRole + 1, location.range.start.line);
    item->setData(Qt::UserRole + 2, location.range.start.character);
  }
}
