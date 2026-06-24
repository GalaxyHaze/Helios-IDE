#include "LspCompletionModel.h"
#include <QAbstractItemView>
#include <QIcon>
#include <QFont>

LspCompletionModel::LspCompletionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LspCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant LspCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const LspCompletionItem &item = m_items[index.row()];

    switch (role) {
    case LabelRole:
        return item.label;
    case KindRole:
        return item.kind;
    case DetailRole:
        return item.detail;
    case InsertTextRole:
        return item.insertText;
    case InsertTextFormatRole:
        return item.insertTextFormat;
    case Qt::ToolTipRole:
        return item.detail;
    default:
        return {};
    }
}

void LspCompletionModel::setItems(const QList<LspCompletionItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

QString LspCompletionModel::insertTextAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return {};
    return m_items[row].insertText;
}

// ── LspCompleter ─────────────────────────────────────────────────────

LspCompleter::LspCompleter(LspCompletionModel *model, QObject *parent)
    : QCompleter(model, parent)
{
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(QCompleter::PopupCompletion);
    setFilterMode(Qt::MatchContains);
    setMaxVisibleItems(12);

    QAbstractItemView *pv = popup();
    pv->setAlternatingRowColors(true);

    connect(this, QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
            this, &LspCompleter::onHighlighted);
}

QString LspCompleter::insertText() const
{
    return m_currentInsertText;
}

int LspCompleter::completionKind() const
{
    return m_currentKind;
}

void LspCompleter::onHighlighted(const QModelIndex &index)
{
    auto *model = qobject_cast<LspCompletionModel *>(completionModel());
    if (!model || !index.isValid())
        return;

    m_currentInsertText = model->data(index, LspCompletionModel::InsertTextRole).toString();
    m_currentKind = model->data(index, LspCompletionModel::KindRole).toInt();
}
