#include <QDebug>
#include <QCheckBox>
#include <QSqlRecord>
#include <QLayoutItem>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QMessageBox>

#include "DxFilterDialog.h"
#include "ui_DxFilterDialog.h"
#include "core/debug.h"
#include "data/Dxcc.h"
#include "core/MembershipQE.h"
#include "DxWidget.h"
#include "core/LogParam.h"
#include "core/PropagationData.h"
#include "data/BandPlan.h"

MODULE_IDENTIFICATION("qlog.ui.dxfilterdialog");

DxFilterDialog::DxFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DxFilterDialog)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    const QList<Band> &bands = BandPlan::bandsList(false, true);
    const QStringList &excludedBandFilter = LogParam::getDXCExcludedBands();
    /********************/
    /* Bands Checkboxes */
    /********************/
    int i = 0;
    for ( const Band &enabledBand : bands )
    {
        const QString &bandName = enabledBand.name;
        QCheckBox *bandCheckbox = new QCheckBox(ui->band_group->parentWidget());
        bandCheckbox->setText(bandName);
        bandCheckbox->setProperty("bandName", bandName); // just to be sure that Bandmap is not translated
        bandCheckbox->setObjectName("bandCheckBox_" + bandName);
        bandCheckbox->setChecked(!excludedBandFilter.contains(bandName));

        int row = i / MAXCOLUMNS;
        int column = i % MAXCOLUMNS;
        ui->band_group->addWidget(bandCheckbox, row, column);
        i++;
    }

    /*********************/
    /* Status Checkboxes */
    /*********************/
    uint statusSetting = LogParam::getDXCFilterDxccStatus();
    ui->newEntitycheckbox->setChecked(statusSetting & DxccStatus::NewEntity);
    ui->newBandcheckbox->setChecked(statusSetting & DxccStatus::NewBand);
    ui->newModecheckbox->setChecked(statusSetting & DxccStatus::NewMode);
    ui->newSlotcheckbox->setChecked(statusSetting & DxccStatus::NewSlot);
    ui->workedcheckbox->setChecked(statusSetting & DxccStatus::Worked);
    ui->confirmedcheckbox->setChecked(statusSetting & DxccStatus::Confirmed);

    /*******************/
    /* Mode Checkboxes */
    /*******************/
    const QString &moderegexp = LogParam::getDXCFilterModeRE();
    ui->cwcheckbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_CW));
    ui->phonecheckbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_PHONE));
    ui->digitalcheckbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_DIGITAL));
    ui->ft8checkbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_FT8));
    ui->ft4checkbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_FT4));
    ui->ft2checkbox->setChecked(moderegexp.contains("|" + BandPlan::MODE_GROUP_STRING_FT2));

    /************************/
    /* Continent Checkboxes */
    /************************/
    const QString &contregexp = LogParam::getDXCFilterContRE();
    ui->afcheckbox->setChecked(contregexp.contains("|AF"));
    ui->ancheckbox->setChecked(contregexp.contains("|AN"));
    ui->ascheckbox->setChecked(contregexp.contains("|AS"));
    ui->eucheckbox->setChecked(contregexp.contains("|EU"));
    ui->nacheckbox->setChecked(contregexp.contains("|NA"));
    ui->occheckbox->setChecked(contregexp.contains("|OC"));
    ui->sacheckbox->setChecked(contregexp.contains("|SA"));

    /********************************/
    /* Spotter Continent Checkboxes */
    /********************************/
    const QString &contregexp_spotter = LogParam::getDXCFilterSpotterContRE();
    ui->afcheckbox_spotter->setChecked(contregexp_spotter.contains("|AF"));
    ui->ancheckbox_spotter->setChecked(contregexp_spotter.contains("|AN"));
    ui->ascheckbox_spotter->setChecked(contregexp_spotter.contains("|AS"));
    ui->eucheckbox_spotter->setChecked(contregexp_spotter.contains("|EU"));
    ui->nacheckbox_spotter->setChecked(contregexp_spotter.contains("|NA"));
    ui->occheckbox_spotter->setChecked(contregexp_spotter.contains("|OC"));
    ui->sacheckbox_spotter->setChecked(contregexp_spotter.contains("|SA"));

    /*****************/
    /* Deduplication */
    /*****************/
    ui->deduplicationGroupBox->setChecked(LogParam::getDXCFilterDedup());
    ui->dedupTimeDiffSpinbox->setValue(LogParam::getDXCFilterDedupTime(DEDUPLICATION_TIME));
    ui->dedupFreqDiffSpinbox->setValue(LogParam::getDXCFilterDedupFreq(DEDUPLICATION_FREQ_TOLERANCE));

    /**********/
    /* MEMBER */
    /**********/

    generateMembershipCheckboxes();

    /*****************************/
    /* Propagation Filter Tab    */
    /*****************************/

    // Minimum Score slider
    ui->minScoreSlider->setValue(LogParam::getDXCFilterMinScore());
    ui->minScoreLabel->setText(QString::number(ui->minScoreSlider->value()));
    connect(ui->minScoreSlider, &QSlider::valueChanged, this, [this](int value) {
        ui->minScoreLabel->setText(QString::number(value));
    });

    // Spotter Sub-Region checkboxes (dynamic)
    ui->spotterRegionGroupBox->setChecked(LogParam::getDXCFilterRegionEnabled());
    generateRegionCheckboxes();

    // Select All / Deselect All buttons
    connect(ui->selectAllRegionsBtn, &QPushButton::clicked, this, [this]() {
        for (QCheckBox *cb : static_cast<const QList<QCheckBox*>&>(regionCheckBoxes))
            cb->setChecked(true);
    });
    connect(ui->deselectAllRegionsBtn, &QPushButton::clicked, this, [this]() {
        for (QCheckBox *cb : static_cast<const QList<QCheckBox*>&>(regionCheckBoxes))
            cb->setChecked(false);
    });

    // Sort by Score
    ui->sortByScoreCheckbox->setChecked(LogParam::getDXCFilterSortByScore());

    // Mutual exclusion: Sub-Region filter supersedes Spotter Continent filter.
    // When Sub-Region groupbox is checked, uncheck all Spotter Continent checkboxes.
    connect(ui->spotterRegionGroupBox, &QGroupBox::toggled, this, [this](bool checked) {
        if (checked)
        {
            ui->afcheckbox_spotter->setChecked(false);
            ui->ancheckbox_spotter->setChecked(false);
            ui->ascheckbox_spotter->setChecked(false);
            ui->eucheckbox_spotter->setChecked(false);
            ui->nacheckbox_spotter->setChecked(false);
            ui->occheckbox_spotter->setChecked(false);
            ui->sacheckbox_spotter->setChecked(false);
        }
        else
        {
            ui->afcheckbox_spotter->setChecked(true);
            ui->ancheckbox_spotter->setChecked(true);
            ui->ascheckbox_spotter->setChecked(true);
            ui->eucheckbox_spotter->setChecked(true);
            ui->nacheckbox_spotter->setChecked(true);
            ui->occheckbox_spotter->setChecked(true);
            ui->sacheckbox_spotter->setChecked(true);
        }
    });
}

void DxFilterDialog::accept()
{
    FCT_IDENTIFICATION;

    /********************/
    /* Bands Checkboxes */
    /********************/
    QStringList excludedBands;
    for ( int i = 0; i < ui->band_group->count(); i++)
    {
        QLayoutItem *item = ui->band_group->itemAt(i);
        if ( !item || !item->widget() ) continue;

        QCheckBox *bandcheckbox = qobject_cast<QCheckBox*>(item->widget());
        if ( bandcheckbox &&  !bandcheckbox->isChecked())
            excludedBands << bandcheckbox->property("bandName").toString();
    }
    LogParam::setDXCExcludedBands(excludedBands);

    /*********************/
    /* Status Checkboxes */
    /*********************/
    uint status = 0;
    if ( ui->newEntitycheckbox->isChecked() ) status |=  DxccStatus::NewEntity;
    if ( ui->newBandcheckbox->isChecked() ) status |=  DxccStatus::NewBand;
    if ( ui->newModecheckbox->isChecked() ) status |=  DxccStatus::NewMode;
    if ( ui->newSlotcheckbox->isChecked() ) status |=  DxccStatus::NewSlot;
    if ( ui->workedcheckbox->isChecked() ) status |=  DxccStatus::Worked;
    if ( ui->confirmedcheckbox->isChecked() ) status |=  DxccStatus::Confirmed;
    LogParam::setDXCFilterDxccStatus(status);

    /*******************/
    /* Mode Checkboxes */
    /*******************/
    QString moderegexp("NOTHING");
    if ( ui->cwcheckbox->isChecked() ) moderegexp.append("|" + BandPlan::MODE_GROUP_STRING_CW);
    if ( ui->phonecheckbox->isChecked() ) moderegexp.append("|" + BandPlan::MODE_GROUP_STRING_PHONE);
    if ( ui->digitalcheckbox->isChecked() ) moderegexp.append("|" + BandPlan::BandPlan::MODE_GROUP_STRING_DIGITAL);
    if ( ui->ft8checkbox->isChecked() ) moderegexp.append("|" + BandPlan::BandPlan::MODE_GROUP_STRING_FT8);
    if ( ui->ft4checkbox->isChecked() ) moderegexp.append("|" + BandPlan::BandPlan::MODE_GROUP_STRING_FT4);
    if ( ui->ft2checkbox->isChecked() ) moderegexp.append("|" + BandPlan::BandPlan::MODE_GROUP_STRING_FT2);
    LogParam::setDXCFilterModeRE(moderegexp);

    /************************/
    /* Continent Checkboxes */
    /************************/
    QString contregexp = "NOTHING";
    if ( ui->afcheckbox->isChecked() ) contregexp.append("|AF");
    if ( ui->ancheckbox->isChecked() ) contregexp.append("|AN");
    if ( ui->ascheckbox->isChecked() ) contregexp.append("|AS");
    if ( ui->eucheckbox->isChecked() ) contregexp.append("|EU");
    if ( ui->nacheckbox->isChecked() ) contregexp.append("|NA");
    if ( ui->occheckbox->isChecked() ) contregexp.append("|OC");
    if ( ui->sacheckbox->isChecked() ) contregexp.append("|SA");
    LogParam::setDXCFilterContRE(contregexp);

    /********************************/
    /* Spotter Continent Checkboxes */
    /********************************/
    QString contregexp_spotter = "NOTHING";
    if ( ui->afcheckbox_spotter->isChecked() ) contregexp_spotter.append("|AF");
    if ( ui->ancheckbox_spotter->isChecked() ) contregexp_spotter.append("|AN");
    if ( ui->ascheckbox_spotter->isChecked() ) contregexp_spotter.append("|AS");
    if ( ui->eucheckbox_spotter->isChecked() ) contregexp_spotter.append("|EU");
    if ( ui->nacheckbox_spotter->isChecked() ) contregexp_spotter.append("|NA");
    if ( ui->occheckbox_spotter->isChecked() ) contregexp_spotter.append("|OC");
    if ( ui->sacheckbox_spotter->isChecked() ) contregexp_spotter.append("|SA");
    LogParam::setDXCFilterSpotterContRE(contregexp_spotter);

    /*****************/
    /* Deduplication */
    /*****************/
    LogParam::setDXCFilterDedup(ui->deduplicationGroupBox->isChecked());
    LogParam::setDXCFilterDedupTime(ui->dedupTimeDiffSpinbox->value() );
    LogParam::setDXCFilterDedupFreq(ui->dedupFreqDiffSpinbox->value());

    /**********/
    /* MEMBER */
    /**********/

    QStringList memberList;

    if ( ui->memberGroupBox->isChecked() )
    {
        memberList.append("DUMMYCLUB");

        for ( QCheckBox* item : static_cast<const QList<QCheckBox*>&>(memberListCheckBoxes) )
            if ( item->isChecked() ) memberList.append(item->text());
    }
    LogParam::setDXCFilterMemberlists(memberList);

    /*****************************/
    /* Propagation Filter Tab    */
    /*****************************/

    // Validate: if sub-region filter is enabled, at least one region must be selected
    if (ui->spotterRegionGroupBox->isChecked())
    {
        bool anyChecked = false;
        for (QCheckBox *cb : static_cast<const QList<QCheckBox*>&>(regionCheckBoxes))
        {
            if (cb->isChecked()) { anyChecked = true; break; }
        }
        if (!anyChecked)
        {
            QMessageBox::warning(this, tr("Propagation Filter"),
                tr("At least one spotter sub-region must be selected."));
            ui->tabWidget->setCurrentWidget(ui->propagation_filters);
            return;
        }
    }

    LogParam::setDXCFilterMinScore(ui->minScoreSlider->value());
    LogParam::setDXCFilterRegionEnabled(ui->spotterRegionGroupBox->isChecked());

    QStringList excludedRegions;
    for (QCheckBox *cb : static_cast<const QList<QCheckBox*>&>(regionCheckBoxes))
    {
        if (!cb->isChecked())
            excludedRegions << cb->text();
    }
    LogParam::setDXCFilterExcludedRegions(excludedRegions);
    LogParam::setDXCFilterSortByScore(ui->sortByScoreCheckbox->isChecked());

    done(QDialog::Accepted);
}

void DxFilterDialog::generateMembershipCheckboxes()
{
    FCT_IDENTIFICATION;

    const QStringList &currentFilter = LogParam::getDXCFilterMemberlists();
    const QStringList &enabledLists = MembershipQE::getEnabledClubLists();

    for ( const QString &enabledClub : enabledLists )
    {
        QCheckBox *columnCheckbox = new QCheckBox(ui->memberGroupBox->parentWidget());
        columnCheckbox->setText(enabledClub);
        columnCheckbox->setChecked(currentFilter.contains(enabledClub));
        memberListCheckBoxes.append(columnCheckbox);
    }

    if ( memberListCheckBoxes.size() == 0 )
    {
        ui->dxMemberGrid->addWidget(new QLabel(tr("No Club List is enabled")));
    }
    else
    {
        int elementIndex = 0;

        for ( QCheckBox* item : static_cast<const QList<QCheckBox*>&>(memberListCheckBoxes) )
        {
            ui->dxMemberGrid->addWidget(item, elementIndex / MAXCOLUMNS, elementIndex % MAXCOLUMNS);
            elementIndex++;
        }
    }

    ui->memberGroupBox->setChecked(!currentFilter.isEmpty());
}

void DxFilterDialog::generateRegionCheckboxes()
{
    FCT_IDENTIFICATION;

    PropagationData *pd = PropagationData::instance();
    QMap<QString, QStringList> regionsByContinent = pd->allRegionsByContinent();

    if (regionsByContinent.isEmpty())
    {
        ui->regionCheckboxLayout->addWidget(
            new QLabel(tr("Propagation data not available yet")));
        return;
    }

    const QStringList &excludedRegions = LogParam::getDXCFilterExcludedRegions();

    // Preferred continent display order (Europe first for typical EU users)
    const QStringList continentOrder = {
        QStringLiteral("Europe"),
        QStringLiteral("Asia"),
        QStringLiteral("Africa"),
        QStringLiteral("North America"),
        QStringLiteral("South America"),
        QStringLiteral("Oceania"),
        QStringLiteral("Pacific")
    };

    bool firstGroup = true;

    for (const QString &continent : continentOrder)
    {
        if (!regionsByContinent.contains(continent))
            continue;

        const QStringList &regions = regionsByContinent.value(continent);

        // Separator between groups
        if (!firstGroup)
        {
            QFrame *line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            ui->regionCheckboxLayout->addWidget(line);
        }
        firstGroup = false;

        // Bold continent header
        QLabel *header = new QLabel(QString("<b>%1</b>").arg(continent));
        ui->regionCheckboxLayout->addWidget(header);

        // Checkboxes in a grid
        QGridLayout *grid = new QGridLayout();
        int idx = 0;
        for (const QString &regionName : regions)
        {
            QCheckBox *cb = new QCheckBox(regionName);
            cb->setChecked(!excludedRegions.contains(regionName));
            regionCheckBoxes.append(cb);
            grid->addWidget(cb, idx / REGION_MAXCOLUMNS, idx % REGION_MAXCOLUMNS);
            idx++;
        }
        ui->regionCheckboxLayout->addLayout(grid);
    }
}

DxFilterDialog::~DxFilterDialog()
{
    FCT_IDENTIFICATION;

    delete ui;
}
