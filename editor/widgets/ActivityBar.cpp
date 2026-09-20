#include "ActivityBar.h"
#include "../core/ThemeManager.h"
#include <QVBoxLayout>
#include <QPainter>
#include <functional>
#include <cmath>
#include <utility>

static QIcon paintIcon(const QColor &color, std::function<void(QPainter &)> draw)
{
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(12, 12);
    p.setPen(QPen(color, 2));
    draw(p);
    p.end();
    return QIcon(pix);
}

static QIcon explorerIcon(const QColor &color)
{
    return paintIcon(color, [](QPainter &p) {
        QPolygonF diamond;
        diamond << QPointF(0, -8) << QPointF(8, 0) << QPointF(0, 8) << QPointF(-8, 0);
        p.drawPolygon(diamond);
    });
}

static QIcon searchIcon(const QColor &color)
{
    return paintIcon(color, [](QPainter &p) {
        p.drawEllipse(QPointF(0, 0), 6, 6);
        QLineF line(QPointF(4, 4), QPointF(9, 9));
        p.drawLine(line);
    });
}

static QIcon gitIcon(const QColor &color)
{
    return paintIcon(color, [](QPainter &p) {
        p.drawLine(QPointF(-4, -8), QPointF(-4, 8));
        p.drawEllipse(QPointF(-4, -7), 2.5, 2.5);
        p.drawEllipse(QPointF(-4, 7), 2.5, 2.5);
        p.drawLine(QPointF(-4, 4), QPointF(4, 4));
        p.drawLine(QPointF(4, -2), QPointF(4, 4));
    });
}

static QIcon settingsIcon(const QColor &color)
{
    return paintIcon(color, [](QPainter &p) {
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
    setObjectName(QStringLiteral("activityBar"));
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ActivityBar::applyTheme);
    applyTheme();

    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setAllowedAreas(Qt::LeftDockWidgetArea);
    setFixedWidth(48);

    auto *titleBar = new QWidget;
    titleBar->setFixedHeight(0);
    setTitleBarWidget(titleBar);

    QWidget *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(4, 8, 4, 8);
    layout->setSpacing(4);

    rebuildIcons();
    m_buttons.append(createButton(m_icons[Explorer], "Explorer (Ctrl+Shift+E)"));
    m_buttons.append(createButton(m_icons[Search], "Search (Ctrl+Shift+F)"));
    m_buttons.append(createButton(m_icons[Git], "Source Control (Ctrl+Shift+G)"));
    m_buttons.append(createButton(m_icons[Settings], "Preferences (Ctrl+,)", false));

    for (int i = 0; i < 3; ++i)
        layout->addWidget(m_buttons[i]);

    layout->addStretch();
    layout->addWidget(m_buttons.back());

    setWidget(content);
    setActiveMode(Explorer);
    applyTheme();
}

QToolButton *ActivityBar::createButton(const QIcon &icon, const QString &tooltip, bool checkable)
{
    auto *btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(22, 22));
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setToolTip(tooltip);
    btn->setFixedSize(40, 40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(checkable);
    btn->setStyleSheet(buttonStyleSheet());

    connect(btn, &QToolButton::clicked, this, [this, btn, checkable]() {
        if (!checkable) {
            emit preferencesRequested();
            return;
        }

        int idx = m_buttons.indexOf(btn);
        if (idx >= 0 && idx < 3)
            // Always emit modeChanged so MainWindow can handle toggling off.
            // We don't call setActiveMode directly because MainWindow will drive
            // the visual checked state update via updateButtonCheckedStates.
            emit modeChanged(Mode(idx));
    });

    return btn;
}

void ActivityBar::setActiveMode(Mode mode, bool sidebarVisible)
{
    QSignalBlocker blocker(this); // Prevent modeChanged from triggering recursively
    m_activeMode = mode;
    for (int i = 0; i < 3 && i < m_buttons.size(); ++i)
        m_buttons[i]->setChecked(sidebarVisible && (i == int(mode)));
    blocker.unblock();
    // We intentionally do not emit modeChanged here, as MainWindow drives this.
}

void ActivityBar::setButtonToolTip(Mode mode, const QString &tooltip)
{
    const int index = int(mode);
    if (index < 0 || index >= m_buttons.size())
        return;
    m_buttons[index]->setToolTip(tooltip);
}

void ActivityBar::applyTheme()
{
    auto &tm = ThemeManager::instance();
    const QColor bg = tm.semanticColor(ThemeManager::SemanticRole::Surface);
    const QColor border = tm.semanticColor(ThemeManager::SemanticRole::Border);

    setStyleSheet(QString("ActivityBar { background: %1; }")
        .arg(bg.name()));

    const QString style = buttonStyleSheet();
    for (QToolButton *button : std::as_const(m_buttons))
        button->setStyleSheet(style);

    rebuildIcons();
    const int count = qMin(m_icons.size(), m_buttons.size());
    for (int i = 0; i < count; ++i)
        m_buttons[i]->setIcon(m_icons[i]);
}

void ActivityBar::rebuildIcons()
{
    const QColor color =
        ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Text);
    m_icons = {
        explorerIcon(color),
        searchIcon(color),
        gitIcon(color),
        settingsIcon(color)
    };
}

QString ActivityBar::buttonStyleSheet()
{
    auto &tm = ThemeManager::instance();
    const QColor hoverBg = tm.semanticColor(ThemeManager::SemanticRole::Hover);
    const QColor hoverBorder =
        tm.semanticColor(ThemeManager::SemanticRole::BorderStrong);
    const QColor checkedBg =
        tm.semanticColor(ThemeManager::SemanticRole::Selected);
    const QColor checkedBorder =
        tm.semanticColor(ThemeManager::SemanticRole::Accent);

    return QString(
        "QToolButton { background: transparent; border: 1px solid transparent; border-radius: 8px; }"
        "QToolButton:hover { background: %1; border-color: %2; }"
        "QToolButton:checked { background: %3; border-color: %4; }")
        .arg(hoverBg.name(), hoverBorder.name(), checkedBg.name(), checkedBorder.name());
}
