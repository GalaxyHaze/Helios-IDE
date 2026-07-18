#include "BreadcrumbsBar.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

BreadcrumbsBar::BreadcrumbsBar(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(12, 2, 12, 2);
    m_layout->setSpacing(0);
    setFixedHeight(22);
    hide();
}

void BreadcrumbsBar::setPath(const QString &filePath, const QString &function)
{
    if (filePath.isEmpty()) {
        clear();
        return;
    }

    QFileInfo fi(filePath);
    QStringList dirs;
    QDir d = fi.dir();
    do {
        QString name = d.dirName();
        if (!name.isEmpty())
            dirs.prepend(name);
#ifdef _WIN32
        // Add drive letter for root dir
        if (d.isRoot())
        {
            QString drivePath = d.path();
            const int driveNameLength = 2;
            if(drivePath.length() > driveNameLength) {
                drivePath = drivePath.left(driveNameLength);
            }
            dirs.prepend(drivePath);
        }
#endif
    } while (d.cdUp());

    rebuild(dirs, fi.fileName(), function);
    show();
}

void BreadcrumbsBar::clear()
{
    for (int i = m_layout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = m_layout->takeAt(i);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    hide();
}

void BreadcrumbsBar::rebuild(const QStringList &dirs, const QString &file, const QString &func)
{
    for (int i = m_layout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = m_layout->takeAt(i);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const int cjkExtraHeight = 7;

    auto addSep = [this]() {
        auto *sep = new QLabel("›");
        sep->setStyleSheet("color: #6c7086; padding: 0 4px; font-size: 11px;");
        m_layout->addWidget(sep);
    };

    auto addLabel = [this](const QString &text, const QString &color, bool useLargeFont, bool bold = false) {
        auto *label = new QLabel(text);
        int fontSize = 11 + (useLargeFont ? cjkExtraHeight : 0);
        QString style = QString("color: %1; font-size: %2px; padding: 0 2px;").arg(color, QString::number(fontSize));
        if (bold)
            style += " font-weight: bold;";
        label->setStyleSheet(style);
        m_layout->addWidget(label);
    };

    bool hasCJK = false;
    QRegularExpression cjkRegex("[\u4E00-\u9FFF\u3400-\u4DBF\u3000-\u303F\uFF00-\uFFEF]");
    for (const QString &dir : dirs) {
        if (dir.contains(cjkRegex)) {
            hasCJK = true;
        }
    }
    if(file.contains(cjkRegex)) {
        hasCJK = true;
    }

    setFixedHeight(22 + (hasCJK ? cjkExtraHeight : 0));

    for (const QString &dir : dirs) {
        addLabel(dir, "#9ca0b0", hasCJK);
        addSep();
    }
    addLabel(file, "#c6d0f5", hasCJK, true);

    if (!func.isEmpty()) {
        addSep();
        addLabel(func, "#e5c890", hasCJK);
    }

    m_layout->addStretch();
}
