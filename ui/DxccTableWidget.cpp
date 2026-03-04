#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QDate>
#include <QSqlError>
#include <QSqlQuery>
#include <QContextMenuEvent>
#include <QAction>
#include "models/DxccTableModel.h"
#include "DxccTableWidget.h"
#include "core/debug.h"
#include "data/StationProfile.h"
#include "data/BandPlan.h"

MODULE_IDENTIFICATION("qlog.ui.dxcctablewidget");

DxccTableWidget::DxccTableWidget(QWidget *parent) : QTableView(parent),
    hasLastQuery(false)
{
    FCT_IDENTIFICATION;

    dxccTableModel = new DxccTableModel(parent);

    this->setObjectName("dxccTableView");
    this->setModel(dxccTableModel);
    this->verticalHeader()->setVisible(false);
    this->setContextMenuPolicy(Qt::DefaultContextMenu);
}

void DxccTableWidget::clear()
{
    FCT_IDENTIFICATION;

    dxccTableModel->clear();
    dxccTableModel->setQuery(QString());
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    show();
}

void DxccTableWidget::updateDxTable(const QString &condition,
                                    const QVariant &conditionValue,
                                    const Band &highlightedBand)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << condition << conditionValue;

    // Cache for refresh after mode toggle
    lastCondition = condition;
    lastConditionValue = conditionValue;
    lastHighlightedBand = highlightedBand;
    hasLastQuery = true;

    const QList<Band>& dxccBands = BandPlan::bandsList(false, true);

    if ( dxccBands.isEmpty() )
        return;

    QString filter(QLatin1String("1 = 1"));
    StationProfile profile = StationProfilesManager::instance()->getCurProfile1();
    QStringList stmt_band_part1;
    QStringList stmt_band_part2;

    if ( profile != StationProfile() )
        filter.append(QString(" AND c.my_dxcc = %1").arg(profile.dxcc));

    for ( const Band &band : dxccBands )
    {
        stmt_band_part1 << QString(" MAX(CASE WHEN band = '%0' THEN  CASE WHEN (eqsl_qsl_rcvd = 'Y') THEN 2 ELSE 1 END  ELSE 0 END) as '%0_eqsl',"
                                   " MAX(CASE WHEN band = '%0' THEN  CASE WHEN (lotw_qsl_rcvd = 'Y') THEN 2 ELSE 1 END  ELSE 0 END) as '%0_lotw',"
                                   " MAX(CASE WHEN band = '%0' THEN  CASE WHEN (qsl_rcvd = 'Y')      THEN 2 ELSE 1 END  ELSE 0 END) as '%0_paper' ")
                                  .arg(band.name);
        stmt_band_part2 << QString(" c.'%0_eqsl' || c.'%0_lotw'|| c.'%0_paper' as '%0'").arg(band.name);
    }

    // Build mode filter for hidden modes
    QStringList hidden = hiddenModes();
    QString modeFilter;
    if ( !hidden.isEmpty() )
    {
        QStringList quoted;
        for ( const QString &mode : hidden )
            quoted << QString("'%1'").arg(mode);
        modeFilter = QString(" WHERE dxcc NOT IN (%1)").arg(quoted.join(","));
    }

    QString stmt = QString("WITH dxcc_summary AS "
                           "             ("
                           "			  SELECT  "
                           "			  m.dxcc , "
                           "		      %1 "
                           "		      FROM contacts c"
                           "		           LEFT OUTER JOIN modes m on c.mode = m.name"
                           "		      WHERE %2 AND %3 GROUP BY m.dxcc ) "
                           " SELECT m.dxcc,"
                           "	   %4 "
                           " FROM (SELECT DISTINCT dxcc"
                           "	   FROM modes%5) m"
                           "        LEFT OUTER JOIN dxcc_summary c ON c.dxcc = m.dxcc "
                           " ORDER BY m.dxcc").arg(stmt_band_part1.join(","),
                                                   filter,
                                                   condition.arg(conditionValue.toString()),
                                                   stmt_band_part2.join(","),
                                                   modeFilter);

    qCDebug(runtime) << stmt;

    dxccTableModel->setQuery(stmt);

    // get default Brush from Mode column - Mode Column has always the default color
    const QVariant &defaultBrush = dxccTableModel->headerData(0, Qt::Horizontal, Qt::BackgroundRole);

    dxccTableModel->setHeaderData(0, Qt::Horizontal, tr("Mode"));

    for ( int i = 0; i < dxccBands.size(); i++ )
    {
        dxccTableModel->setHeaderData(i+1, Qt::Horizontal, ( highlightedBand == dxccBands.at(i) ) ? QBrush(Qt::darkGray)
                                                                                               : defaultBrush, Qt::BackgroundRole);
        dxccTableModel->setHeaderData(i+1, Qt::Horizontal, dxccBands.at(i).name);
    }
    setColumnWidth(0,65);

    // Resize table height to fit content (header + visible rows)
    int height = horizontalHeader()->height();
    for ( int i = 0; i < dxccTableModel->rowCount(); i++ )
        height += rowHeight(i);
    height += 2; // frame border
    setFixedHeight(height);

    show();
}

void DxccTableWidget::setDxCallsign(const QString &dxCallsign, const Band &band)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << dxCallsign;

    if (!dxCallsign.isEmpty())
        updateDxTable("c.callsign = '%1'", dxCallsign.toUpper(), band);
    else
        clear();

}

void DxccTableWidget::setDxcc(int dxcc, const Band &highlightedBand)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << dxcc;

    if ( dxcc )
        updateDxTable("c.dxcc = %1", dxcc, highlightedBand);
    else
        clear();
}

void DxccTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    FCT_IDENTIFICATION;

    QMenu menu(this);
    menu.setTitle(tr("Visible Modes"));

    QStringList allModes = {"CW", "DIGITAL", "PHONE"};
    QStringList hidden = hiddenModes();

    for ( const QString &mode : allModes )
    {
        QAction *action = menu.addAction(mode);
        action->setCheckable(true);
        action->setChecked(!hidden.contains(mode));
    }

    QAction *selected = menu.exec(event->globalPos());
    if ( !selected )
        return;

    QString mode = selected->text();
    if ( selected->isChecked() )
        hidden.removeAll(mode);
    else
        hidden.append(mode);

    setHiddenModes(hidden);

    // Refresh the table with updated mode filter
    if ( hasLastQuery )
        updateDxTable(lastCondition, lastConditionValue, lastHighlightedBand);
}

QStringList DxccTableWidget::hiddenModes() const
{
    QSettings settings;
    return settings.value("dxcctable/hiddenmodes").toStringList();
}

void DxccTableWidget::setHiddenModes(const QStringList &modes)
{
    QSettings settings;
    settings.setValue("dxcctable/hiddenmodes", modes);
}
