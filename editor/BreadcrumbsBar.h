#ifndef BREADCRUMBSBAR_H
#define BREADCRUMBSBAR_H

#include <QWidget>
#include <QStringList>

class QLabel;
class QHBoxLayout;

class BreadcrumbsBar : public QWidget
{
    Q_OBJECT

public:
    explicit BreadcrumbsBar(QWidget *parent = nullptr);

    void setPath(const QString &filePath, const QString &function = {});
    void clear();

private:
    void rebuild(const QStringList &dirs, const QString &file, const QString &func);

    QHBoxLayout *m_layout;
};

#endif
