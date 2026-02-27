#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDockWidget>
#include <QResizeEvent>

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

    // Connect to data source
    connect(PropagationData::instance(), &PropagationData::dataUpdated,
            this, &PropagationWidget::updateDisplay);

    // Initial draw (may be empty until first fetch completes)
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

    // Main vertical layout wrapping title + chips
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 2, 4, 2);
    mainLayout->setSpacing(2);

    // Title label — "Best Band from {region}"
    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() - 1);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Status label (shown while loading)
    statusLabel = new QLabel(tr("Loading propagation data…"), this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(statusLabel);

    // Chip container — direction determined by dock orientation
    chipLayout = new QHBoxLayout();
    chipLayout->setContentsMargins(0, 0, 0, 0);
    chipLayout->setSpacing(4);
    mainLayout->addLayout(chipLayout);

    updateTitle();
}

void PropagationWidget::rebuildChips()
{
    FCT_IDENTIFICATION;

    // Clear existing chips
    for (auto &chip : chips)
    {
        chipLayout->removeWidget(chip.container);
        delete chip.container;
    }
    chips.clear();

    const auto &corridors = PropagationData::instance()->corridors();
    const auto &order = corridorOrder();

    for (const QString &region : order)
    {
        ChipWidget cw;

        // Container with border
        cw.container = new QWidget(this);
        cw.container->setMinimumWidth(50);

        QVBoxLayout *chipVBox = new QVBoxLayout(cw.container);
        chipVBox->setContentsMargins(4, 3, 4, 3);
        chipVBox->setSpacing(1);
        chipVBox->setAlignment(Qt::AlignCenter);

        // Region label (AF, SA, NA, …)
        cw.regionLabel = new QLabel(region, cw.container);
        cw.regionLabel->setAlignment(Qt::AlignCenter);
        QFont rf = cw.regionLabel->font();
        rf.setBold(true);
        rf.setPointSize(rf.pointSize() - 1);
        cw.regionLabel->setFont(rf);
        chipVBox->addWidget(cw.regionLabel);

        // Band pill
        cw.bandLabel = new QLabel(cw.container);
        cw.bandLabel->setAlignment(Qt::AlignCenter);
        QFont bf = cw.bandLabel->font();
        bf.setPointSize(bf.pointSize() - 1);
        cw.bandLabel->setFont(bf);
        chipVBox->addWidget(cw.bandLabel);

        // Populate from data
        auto it = corridors.constFind(region);
        if (it != corridors.constEnd() && !it->bestBand.isEmpty())
        {
            cw.bandLabel->setText(it->bestBand);
            cw.bandLabel->setStyleSheet(bandPillStyleSheet(it->bestBand));
            cw.container->setStyleSheet(chipStyleSheet(it->index, it->bestBand));
        }
        else
        {
            cw.bandLabel->setText(QStringLiteral("---"));
            cw.bandLabel->setStyleSheet(
                "background-color: #e0e0e0; color: #999; "
                "border-radius: 3px; padding: 1px 4px;");
            cw.container->setStyleSheet(chipStyleSheet(-1, QString()));
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
        titleLabel->setText(tr("Best Band from %1").arg(region));

    // Also update parent dock widget title if available
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

    // Switch layout direction based on aspect ratio
    QBoxLayout::Direction newDir = isHorizontal()
        ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;

    if (chipLayout->direction() != newDir)
        chipLayout->setDirection(newDir);
}

bool PropagationWidget::event(QEvent *event)
{
    // Catch dock location changes
    if (event->type() == QEvent::ParentChange ||
        event->type() == QEvent::Move)
    {
        updateTitle();
    }
    return QWidget::event(event);
}

// =========================================================================
//  Styling
// =========================================================================

QString PropagationWidget::bandColor(const QString &band)
{
    // Band → pill background color
    if (band == QLatin1String("10m"))  return QStringLiteral("#4287f5");  // blue
    if (band == QLatin1String("12m"))  return QStringLiteral("#42a5f5");  // light blue
    if (band == QLatin1String("15m"))  return QStringLiteral("#26a69a");  // teal
    if (band == QLatin1String("17m"))  return QStringLiteral("#66bb6a");  // light green
    if (band == QLatin1String("20m"))  return QStringLiteral("#43a047");  // green
    if (band == QLatin1String("30m"))  return QStringLiteral("#7e57c2");  // deep purple
    if (band == QLatin1String("40m"))  return QStringLiteral("#ab47bc");  // purple
    if (band == QLatin1String("60m"))  return QStringLiteral("#ef5350");  // light red
    if (band == QLatin1String("80m"))  return QStringLiteral("#e53935");  // red
    if (band == QLatin1String("160m")) return QStringLiteral("#c62828");  // dark red
    return QStringLiteral("#9e9e9e");  // grey fallback
}

QString PropagationWidget::borderColor(int index)
{
    if (index >= 50)  return QStringLiteral("#43a047");  // green — open
    if (index >= 10)  return QStringLiteral("#ff9800");  // orange — marginal
    return QStringLiteral("#bdbdbd");                     // grey — closed / no data
}

QString PropagationWidget::chipStyleSheet(int index, const QString & /* band */)
{
    QString bc = borderColor(index);
    return QString(
        "QWidget { "
        "  border: 2px solid %1; "
        "  border-radius: 6px; "
        "  background-color: palette(window); "
        "}").arg(bc);
}

QString PropagationWidget::bandPillStyleSheet(const QString &band)
{
    QString bg = bandColor(band);
    return QString(
        "background-color: %1; "
        "color: white; "
        "border-radius: 3px; "
        "padding: 1px 6px; "
        "border: none;").arg(bg);
}
