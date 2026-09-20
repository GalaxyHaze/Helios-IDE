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
    setMinimumWidth(180);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel = new QLabel;
    m_titleLabel->setStyleSheet("font-weight: bold; padding: 6px 10px; font-size: 12px;");
    layout->addWidget(m_titleLabel);

    m_emptyLabel = new QLabel;
    m_emptyLabel->setWordWrap(true);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->hide();
    layout->addWidget(m_emptyLabel);

    m_treeWidget = new QTreeWidget;
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setIndentation(12);
    m_treeWidget->setAnimated(true);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->setStyleSheet({});
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
    m_emptyLabel->show();
    m_treeWidget->hide();
}

void OutlinePanel::setSymbols(const QJsonArray &symbols)
{
    m_treeWidget->clear();
    if (symbols.isEmpty()) {
        m_emptyLabel->show();
        m_treeWidget->hide();
        return;
    }
    m_emptyLabel->hide();
    m_treeWidget->show();
    populateTree(symbols, nullptr);
    m_treeWidget->expandAll();
}

void OutlinePanel::populateTree(const QJsonArray &symbols, QTreeWidgetItem *parentItem)
{
    for (const QJsonValue &val : symbols) {
        QJsonObject obj = val.toObject();
        QString name = obj.value("name").toString();
        int kind = obj.value("kind").toInt();

        QJsonObject selectionRangeObj =
            obj.value("selectionRange").toObject();
        QJsonObject rangeObj = selectionRangeObj.isEmpty()
                                   ? obj.value("range").toObject()
                                   : selectionRangeObj;
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
    
    setPalette(tm.palette());

    const QString textHex = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString bgHex = tm.semanticColor(ThemeManager::SemanticRole::Surface).name();
    const QString windowTextHex = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
    const QString borderHex = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
    const QString itemHoverHex = tm.semanticColor(ThemeManager::SemanticRole::Hover).name();
    const QString itemSelectedHex = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
    const QString itemSelectedTextHex = tm.semanticColor(ThemeManager::SemanticRole::SelectedText).name();

    m_titleLabel->setText(
        TranslationManager::instance().translate("outline.title"));
    m_emptyLabel->setText(
        TranslationManager::instance().translate("outline.no_symbols"));
    m_titleLabel->setStyleSheet(
        QString("font-weight: bold; padding: 6px 10px; font-size: 12px; border-bottom: 1px solid %1; color: %2; background: %3;")
        .arg(borderHex, windowTextHex, bgHex)
    );
    m_emptyLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; padding: 16px 10px; background: %2;")
            .arg(windowTextHex, bgHex));

    m_treeWidget->setStyleSheet(
        QString(
            "QTreeWidget { background: %1; color: %2; border: none; font-size: 12px; }"
            "QTreeWidget::item { padding: 7px 10px; color: %2; border-radius: 6px; }"
            "QTreeWidget::item:hover { background: %3; }"
            "QTreeWidget::item:selected { background: %4; color: %5; }"
        ).arg(bgHex, textHex, itemHoverHex, itemSelectedHex, itemSelectedTextHex)
    );
}
