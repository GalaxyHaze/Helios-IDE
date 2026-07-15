#include "OutlinePanel.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QJsonObject>
#include <QHeaderView>

OutlinePanel::OutlinePanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel = new QLabel("Structure");
    m_titleLabel->setStyleSheet("font-weight: bold; padding: 6px 10px; font-size: 12px; border-bottom: 1px solid #1e1e2e;");
    layout->addWidget(m_titleLabel);

    m_treeWidget = new QTreeWidget;
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setIndentation(12);
    m_treeWidget->setAnimated(true);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->setStyleSheet(
        "QTreeWidget { background: #11111b; color: #a5adce; border: none; font-size: 12px; }"
        "QTreeWidget::item { padding: 4px 6px; }"
        "QTreeWidget::item:hover { background: #363a4f; }"
        "QTreeWidget::item:selected { background: #363a4f; color: #c6d0f5; }"
    );
    layout->addWidget(m_treeWidget);

    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &OutlinePanel::handleItemActivated);
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &OutlinePanel::handleItemActivated);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &OutlinePanel::updateThemeAndLanguage);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged, this, &OutlinePanel::updateThemeAndLanguage);

    updateThemeAndLanguage();
}

void OutlinePanel::clear()
{
    m_treeWidget->clear();
}

void OutlinePanel::setSymbols(const QJsonArray &symbols)
{
    m_treeWidget->clear();
    populateTree(symbols, nullptr);
    m_treeWidget->expandAll();
}

void OutlinePanel::populateTree(const QJsonArray &symbols, QTreeWidgetItem *parentItem)
{
    for (const QJsonValue &val : symbols) {
        QJsonObject obj = val.toObject();
        QString name = obj.value("name").toString();
        int kind = obj.value("kind").toInt();

        QJsonObject rangeObj = obj.value("range").toObject();
        QJsonObject startObj = rangeObj.value("start").toObject();
        int line = startObj.value("line").toInt();
        int col = startObj.value("character").toInt();

        auto *item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setData(0, Qt::UserRole, line);
        item->setData(0, Qt::UserRole + 1, col);

        QIcon icon;
        if (kind == 12 || kind == 6) {
            icon = QIcon::fromTheme("dialog-information");
        } else if (kind == 5 || kind == 11) {
            icon = QIcon::fromTheme("applications-engineering");
        } else {
            icon = QIcon::fromTheme("text-x-generic");
        }
        item->setIcon(0, icon);

        if (parentItem) {
            parentItem->addChild(item);
        } else {
            m_treeWidget->addTopLevelItem(item);
        }

        if (obj.contains("children")) {
            populateTree(obj.value("children").toArray(), item);
        }
    }
}

void OutlinePanel::handleItemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;

    bool okLine = false;
    int line = item->data(0, Qt::UserRole).toInt(&okLine);
    int col = item->data(0, Qt::UserRole + 1).toInt();

    if (okLine) {
        emit symbolSelected(line, col);
    }
}

void OutlinePanel::updateThemeAndLanguage()
{
    auto &tm = ThemeManager::instance();
    
    QPalette pal = tm.palette();
    setPalette(pal);

    QColor base = pal.color(QPalette::Base);
    QColor text = pal.color(QPalette::Text);
    QColor windowText = pal.color(QPalette::WindowText);

    QString textHex = text.name();
    QString bgHex = base.name();
    QString windowTextHex = windowText.name();
    QString borderHex = tm.customColor("sidebarBorder", QColor("#363a4f")).name();
    QString itemHoverHex = tm.customColor("treeHover", QColor("#363a4f")).name();

    m_titleLabel->setText("Structure");
    m_titleLabel->setStyleSheet(
        QString("font-weight: bold; padding: 6px 10px; font-size: 12px; border-bottom: 1px solid %1; color: %2; background: %3;")
        .arg(borderHex, windowTextHex, bgHex)
    );

    m_treeWidget->setStyleSheet(
        QString(
            "QTreeWidget { background: %1; color: %2; border: none; font-size: 12px; }"
            "QTreeWidget::item { padding: 4px 6px; color: %2; }"
            "QTreeWidget::item:hover { background: %3; }"
            "QTreeWidget::item:selected { background: %3; color: %4; }"
        ).arg(bgHex, textHex, itemHoverHex, windowTextHex)
    );
}
