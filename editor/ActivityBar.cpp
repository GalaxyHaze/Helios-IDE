#include "ActivityBar.h"
#include <QVBoxLayout>
#include <QPainter>
#include <cmath>

static QIcon paintIcon(std::function<void(QPainter &)> draw)
{
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(12, 12);
    draw(p);
    p.end();
    return QIcon(pix);
}

static QColor icFG() { return QColor("#c6d0f5"); }
static QPen icPen() { return QPen(icFG(), 2); }

static QIcon explorerIcon()
{
    return paintIcon([](QPainter &p) {
        p.setPen(icPen());
        QPolygonF diamond;
        diamond << QPointF(0, -8) << QPointF(8, 0) << QPointF(0, 8) << QPointF(-8, 0);
        p.drawPolygon(diamond);
    });
}

static QIcon searchIcon()
{
    return paintIcon([](QPainter &p) {
        p.setPen(icPen());
        p.drawEllipse(QPointF(0, 0), 6, 6);
        QLineF line(QPointF(4, 4), QPointF(9, 9));
        p.drawLine(line);
    });
}

static QIcon gitIcon()
{
    return paintIcon([](QPainter &p) {
        p.setPen(icPen());
        p.drawLine(QPointF(-4, -8), QPointF(-4, 8));
        p.drawEllipse(QPointF(-4, -7), 2.5, 2.5);
        p.drawEllipse(QPointF(-4, 7), 2.5, 2.5);
        p.drawLine(QPointF(-4, 4), QPointF(4, 4));
        p.drawLine(QPointF(4, -2), QPointF(4, 4));
    });
}

static QIcon settingsIcon()
{
    return paintIcon([](QPainter &p) {
        p.setPen(icPen());
        p.drawEllipse(QPointF(0, 0), 3, 3);
        for (int i = 0; i < 8; ++i) {
            qreal a = i * M_PI / 4;
            QPointF outer(cos(a) * 9, sin(a) * 9);
            QPointF inner(cos(a) * 6, sin(a) * 6);
            p.drawLine(outer, inner);
        }
    });
}

ActivityBar::ActivityBar(QWidget *parent)
    : QDockWidget(parent)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setAllowedAreas(Qt::LeftDockWidgetArea);
    setFixedWidth(48);

    auto *titleBar = new QWidget;
    titleBar->setFixedHeight(0);
    setTitleBarWidget(titleBar);

    QWidget *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->setSpacing(2);

    m_buttons.append(createButton(explorerIcon(), "Explorer (Ctrl+B)"));
    m_buttons.append(createButton(searchIcon(), "Search (Ctrl+Shift+F)"));
    m_buttons.append(createButton(gitIcon(), "Source Control (Ctrl+Shift+G)"));

    layout->addStretch();

    m_buttons.append(createButton(settingsIcon(), "Settings (Ctrl+,)"));

    for (auto *btn : m_buttons)
        layout->addWidget(btn);

    setWidget(content);
    setActiveMode(Explorer);

    setStyleSheet(
        "ActivityBar { background: #11111b; border-right: 1px solid #1e1e2e; }"
    );
}

QToolButton *ActivityBar::createButton(const QIcon &icon, const QString &tooltip)
{
    auto *btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(22, 22));
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setToolTip(tooltip);
    btn->setFixedSize(40, 40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    btn->setStyleSheet(
        "QToolButton { background: transparent; border: none; border-radius: 6px; }"
        "QToolButton:hover { background: #363a4f; }"
        "QToolButton:checked { background: #1e1e2e; }"
    );

    connect(btn, &QToolButton::clicked, this, [this, btn]() {
        int idx = m_buttons.indexOf(btn);
        if (idx >= 0 && idx < 4)
            setActiveMode(Mode(idx));
    });

    return btn;
}

void ActivityBar::setActiveMode(Mode mode)
{
    m_activeMode = mode;
    for (int i = 0; i < m_buttons.size(); ++i)
        m_buttons[i]->setChecked(i == int(mode));
    emit modeChanged(mode);
}
