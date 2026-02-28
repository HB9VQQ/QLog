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

    // Title
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

    if (region.isEmpty())
        titleLabel->setText(tr("Propagation"));
    else
        titleLabel->setText(tr("Best Band from %1 to").arg(region));

    QDockWidget *dock = qobject_cast<QDockWidget *>(parentWidget());
    if (dock)
        dock->setWindowTitle(titleLabel->text());
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
    if (band == QLatin1String("10m"))  return QColor(66, 135, 245);
    if (band == QLatin1String("12m"))  return QColor(66, 165, 245);
    if (band == QLatin1String("15m"))  return QColor(38, 166, 154);
    if (band == QLatin1String("17m"))  return QColor(102, 187, 106);
    if (band == QLatin1String("20m"))  return QColor(67, 160, 71);
    if (band == QLatin1String("30m"))  return QColor(126, 87, 194);
    if (band == QLatin1String("40m"))  return QColor(171, 71, 188);
    if (band == QLatin1String("60m"))  return QColor(239, 83, 80);
    if (band == QLatin1String("80m"))  return QColor(229, 57, 53);
    if (band == QLatin1String("160m")) return QColor(198, 40, 40);
    return qApp->palette().color(QPalette::Text);
}
