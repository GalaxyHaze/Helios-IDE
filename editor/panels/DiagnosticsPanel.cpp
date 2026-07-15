#include "DiagnosticsPanel.h"
#include "../core/ThemeManager.h"
#include <QFileInfo>
#include <QFont>
#include <QUrl>
#include <QVBoxLayout>

DiagnosticsPanel::DiagnosticsPanel(QWidget *parent)
    : QDockWidget("Diagnostics", parent) {
  setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

  m_list = new QListWidget;
  m_list->setFont(QFont("monospace", 10));
  m_list->setWordWrap(true);

  setWidget(m_list);

  connect(m_list, &QListWidget::itemClicked, this,
          &DiagnosticsPanel::onItemClicked);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &DiagnosticsPanel::updateTheme);
  updateTheme();
}

void DiagnosticsPanel::updateTheme() {
  auto &tm = ThemeManager::instance();
  const QPalette pal = tm.palette();
  setPalette(pal);

  m_list->setStyleSheet(
      QString(
          "QListWidget { background: %1; color: %2; border: none; }"
          "QListWidget::item { padding: 2px 4px; border-bottom: 1px solid %3; }"
          "QListWidget::item:selected { background: %4; color: %5; }"
          "QListWidget::item:alternate { background: %6; }")
          .arg(pal.color(QPalette::Base).name(),
               pal.color(QPalette::Text).name(),
               tm.customColor("sidebarBorder", QColor("#272d3d")).name(),
               tm.customColor("treeSelected", pal.color(QPalette::Highlight))
                   .name(),
               tm.customColor("treeSelectedFg",
                              pal.color(QPalette::HighlightedText))
                   .name(),
               pal.color(QPalette::AlternateBase).name()));
}

void DiagnosticsPanel::clear() {
  m_diagnosticsByUri.clear();
  m_list->clear();
  m_errorCount = 0;
  m_warningCount = 0;
  emit countsChanged(0, 0);
}

void DiagnosticsPanel::setDiagnostics(const QString &uri, int version,
                                      const QList<LspDiagnostic> &diagnostics) {
  Q_UNUSED(version)
  m_diagnosticsByUri.insert(uri, diagnostics);
  rebuild();
}

void DiagnosticsPanel::clearDiagnostics(const QString &uri) {
  m_diagnosticsByUri.remove(uri);
  rebuild();
}

void DiagnosticsPanel::rebuild() {
  m_list->clear();
  m_errorCount = 0;
  m_warningCount = 0;
  auto &tm = ThemeManager::instance();

  for (auto it = m_diagnosticsByUri.cbegin(); it != m_diagnosticsByUri.cend();
       ++it) {
    const QString &uri = it.key();
    for (const LspDiagnostic &d : it.value()) {
      QString severity;
      QColor color;
      switch (d.severity) {
      case 1:
        severity = "E";
        color = tm.customColor("diagnosticError", QColor("#ff7a90"));
        m_errorCount++;
        break;
      case 2:
        severity = "W";
        color = tm.customColor("diagnosticWarning", QColor("#f1c77a"));
        m_warningCount++;
        break;
      case 3:
        severity = "I";
        color = tm.customColor("diagnosticInfo", QColor("#70c9f0"));
        break;
      default:
        severity = "?";
        color = tm.customColor("diagnosticUnknown", QColor("#aeb8ce"));
        break;
      }

      QFileInfo fi(QUrl(uri).toLocalFile());
      QString text =
          QString("[%1] %2:%3:%4 - %5")
              .arg(severity, fi.fileName(),
                   QString::number(d.range.start.line + 1),
                   QString::number(d.range.start.character + 1), d.message);

      auto *item = new QListWidgetItem(text);
      item->setForeground(color);
      item->setData(Qt::UserRole, uri);
      item->setData(Qt::UserRole + 1, d.range.start.line);
      item->setData(Qt::UserRole + 2, d.range.start.character);
      m_list->addItem(item);
    }
  }

  emit countsChanged(m_errorCount, m_warningCount);
}

void DiagnosticsPanel::onItemClicked(QListWidgetItem *item) {
  if (!item)
    return;

  QString uri = item->data(Qt::UserRole).toString();
  int line = item->data(Qt::UserRole + 1).toInt();
  int character = item->data(Qt::UserRole + 2).toInt();

  emit navigateToLocation(uri, line, character);
}
