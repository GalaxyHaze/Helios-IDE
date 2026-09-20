#include "ReferencesPanel.h"
#include "../core/ThemeManager.h"

#include <QFileInfo>
#include <QFont>
#include <QVBoxLayout>
#include <QUrl>

ReferencesPanel::ReferencesPanel(QWidget *parent)
    : QWidget(parent) {
  setObjectName(QStringLiteral("referencesDock"));
  m_list = new QListWidget(this);
  m_list->setFont(QFont("monospace", 10));
  m_list->setWordWrap(true);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_list);
  connect(m_list, &QListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
            emit navigateToLocation(item->data(Qt::UserRole).toString(),
                                    item->data(Qt::UserRole + 1).toInt(),
                                    item->data(Qt::UserRole + 2).toInt());
          });
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
          this, &ReferencesPanel::applyTheme);
  applyTheme();
}

void ReferencesPanel::applyTheme()
{
  auto &tm = ThemeManager::instance();
  const QColor canvas =
      tm.semanticColor(ThemeManager::SemanticRole::Canvas);
  const QColor text =
      tm.semanticColor(ThemeManager::SemanticRole::Text);
  const QColor border =
      tm.semanticColor(ThemeManager::SemanticRole::Border);
  const QColor selected =
      tm.semanticColor(ThemeManager::SemanticRole::Selected);
  const QColor selectedText =
      tm.semanticColor(ThemeManager::SemanticRole::SelectedText);
  setStyleSheet(QString("ReferencesPanel { background: %1; }")
                    .arg(canvas.name()));
  m_list->setStyleSheet(
      QString("QListWidget { background: %1; color: %2; border: none; }"
              "QListWidget::item { padding: 4px 6px; border-bottom: 1px solid %3; }"
              "QListWidget::item:selected { background: %4; color: %5; }")
          .arg(canvas.name(), text.name(), border.name(),
               selected.name(), selectedText.name()));
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
