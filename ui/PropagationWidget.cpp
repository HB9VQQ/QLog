#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDockWidget>
#include <QResizeEvent>
#include <QApplication>
#include <QPalette>

#include "PropagationWidget.h"
#include "core/PropagationData.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.propagationwidget");

// =========================================================================
//  Corridor display order
// =========================================================================

const QStringList &PropagationWidget::corridorOrder()
{
    static const QStringList order = {
        QStringLiteral("AF"),
        QStringLiteral("AS"),
        QStringLiteral("NA"),
        QStringLiteral("OC"),
        QStringLiteral("PAC"),
        QStringLiteral("SA")
    };
    return order;
}

// =========================================================================
//  Construction
// =========================================================================

PropagationWidget::PropagationWidget(QWidget *parent)
    : QWidget(parent)
{
    FCT_IDENTIFICATION;

    buildLayout();

    connect(PropagationData::instance(), &PropagationData::dataUpdated,
            this, &PropagationWidget::updateDisplay);

    updateDisplay();
}

PropagationWidget::~PropagationWidget()
{
    FCT_IDENTIFICATION;
}

// =========================================================================
//  Layout management
// =========================================================================

void PropagationWidget::buildLayout()
{
    FCT_IDENTIFICATION;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 2, 4, 2);
    mainLayout->setSpacing(2);

    // Title label (inline, above chips)
    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Status (shown while loading)
    statusLabel = new QLabel(tr("Loading propagation data…"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText,
                 qApp->palette().color(QPalette::Disabled, QPalette::Text));
    statusLabel->setPalette(pal);
    mainLayout->addWidget(statusLabel);

    // Chip container
    chipLayout = new QHBoxLayout();
    chipLayout->setContentsMargins(0, 0, 0, 0);
    chipLayout->setSpacing(8);
    mainLayout->addLayout(chipLayout);

    updateTitle();
}

void PropagationWidget::rebuildChips()
{
    FCT_IDENTIFICATION;

    for (auto &chip : chips)
    {
        chipLayout->removeWidget(chip.container);
        delete chip.container;
    }
    chips.clear();

    const auto &corridors = PropagationData::instance()->corridors();
    const auto &order = corridorOrder();

    QColor textColor = qApp->palette().color(QPalette::Text);
    QColor dimColor  = qApp->palette().color(QPalette::Disabled, QPalette::Text);

    for (const QString &region : order)
    {
        ChipWidget cw;
        cw.container = new QWidget(this);

        QHBoxLayout *hbox = new QHBoxLayout(cw.container);
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->setSpacing(3);

        // Region label — plain text like the rest of QLog
        cw.regionLabel = new QLabel(region, cw.container);
        cw.regionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hbox->addWidget(cw.regionLabel);

        // Band label — colored text only, no background
        cw.bandLabel = new QLabel(cw.container);
        cw.bandLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QFont bf = cw.bandLabel->font();
        bf.setBold(true);
        cw.bandLabel->setFont(bf);
        hbox->addWidget(cw.bandLabel);

        auto it = corridors.constFind(region);
        if (it != corridors.constEnd() && !it->bestBand.isEmpty())
        {
            cw.bandLabel->setText(it->bestBand);
            QPalette bp = cw.bandLabel->palette();
            bp.setColor(QPalette::WindowText, bandColor(it->bestBand));
            cw.bandLabel->setPalette(bp);
        }
        else
        {
            cw.bandLabel->setText(QStringLiteral("---"));
            QPalette bp = cw.bandLabel->palette();
            bp.setColor(QPalette::WindowText, dimColor);
            cw.bandLabel->setPalette(bp);
        }

        chipLayout->addWidget(cw.container);
        chips.append(cw);
    }
}

void PropagationWidget::updateTitle()
{
    FCT_IDENTIFICATION;

    PropagationData *pd = PropagationData::instance();
    QString region = pd->userRegionName();

    QString title = region.isEmpty()
        ? tr("Propagation")
        : tr("Best Band from %1 to").arg(region);

    if (titleLabel)
        titleLabel->setText(title);

    QDockWidget *dock = qobject_cast<QDockWidget *>(parentWidget());
    if (dock)
        dock->setWindowTitle(QString());
}

// =========================================================================
//  Data update slot
// =========================================================================

void PropagationWidget::updateDisplay()
{
    FCT_IDENTIFICATION;

    PropagationData *pd = PropagationData::instance();

    if (!pd->isCorridorsLoaded())
    {
        statusLabel->setVisible(true);
        return;
    }

    statusLabel->setVisible(false);
    updateTitle();
    rebuildChips();
}

// =========================================================================
//  Orientation detection
// =========================================================================

bool PropagationWidget::isHorizontal() const
{
    return width() > height();
}

void PropagationWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!chipLayout)
        return;

    QBoxLayout::Direction newDir = isHorizontal()
        ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;

    if (chipLayout->direction() != newDir)
        chipLayout->setDirection(newDir);
}

bool PropagationWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange ||
        event->type() == QEvent::Move)
    {
        updateTitle();
    }
    return QWidget::event(event);
}

// =========================================================================
//  Band colors
// =========================================================================

QColor PropagationWidget::bandColor(const QString &band)
{
    // Match DX Map colors: green (high freq) to purple (low freq)
    if (band == QLatin1String("10m"))  return QColor(34, 197, 94);    // #22c55e green
    if (band == QLatin1String("12m"))  return QColor(132, 204, 22);   // #84cc16 lime
    if (band == QLatin1String("15m"))  {
        // Yellow is unreadable on light backgrounds
        bool dark = qApp->palette().color(QPalette::Window).lightnessF() < 0.5;
        return dark ? QColor(250, 204, 21) : QColor(184, 150, 15);   // #facc15 / #b8960f
    }
    if (band == QLatin1String("17m"))  return QColor(251, 146, 60);   // #fb923c orange
    if (band == QLatin1String("20m"))  return QColor(249, 115, 22);   // #f97316 deep orange
    if (band == QLatin1String("30m"))  return QColor(239, 68, 68);    // between 20m and 40m
    if (band == QLatin1String("40m"))  return QColor(239, 68, 68);    // #ef4444 red
    if (band == QLatin1String("60m"))  return QColor(192, 75, 175);   // between 40m and 80m
    if (band == QLatin1String("80m"))  return QColor(168, 85, 247);   // #a855f7 purple
    if (band == QLatin1String("160m")) return QColor(139, 92, 246);   // deeper purple
    return qApp->palette().color(QPalette::Text);
}
