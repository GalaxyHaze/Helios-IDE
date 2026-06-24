#ifndef LSPCOMPLETIONMODEL_H
#define LSPCOMPLETIONMODEL_H

#include <QAbstractListModel>
#include <QCompleter>
#include <QStringList>
#include <QList>
#include "LspClient.h"

class LspCompletionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum CompletionRoles {
        LabelRole = Qt::DisplayRole,
        KindRole = Qt::UserRole + 1,
        DetailRole = Qt::UserRole + 2,
        InsertTextRole = Qt::UserRole + 3,
        InsertTextFormatRole = Qt::UserRole + 4
    };

    explicit LspCompletionModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setItems(const QList<LspCompletionItem> &items);
    QString insertTextAt(int row) const;

private:
    QList<LspCompletionItem> m_items;
};

class LspCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit LspCompleter(LspCompletionModel *model, QObject *parent = nullptr);

    QString insertText() const;
    int completionKind() const;

private slots:
    void onHighlighted(const QModelIndex &index);

private:
    QAbstractItemView *m_popup = nullptr;
    mutable int m_currentKind = 0;
    mutable QString m_currentInsertText;
};

#endif // LSPCOMPLETIONMODEL_H
