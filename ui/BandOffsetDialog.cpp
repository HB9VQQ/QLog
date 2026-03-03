#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>

#include "BandOffsetDialog.h"
#include "data/BandPlan.h"
#include <QDoubleSpinBox>
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.bandoffsetdialog");

BandOffsetDialog::BandOffsetDialog(const QMap<QString, double> &offsets,
                                   QWidget *parent)
    : QDialog(parent)
{
    FCT_IDENTIFICATION;

    setWindowTitle(tr("Per-Band Azimuth Offsets"));
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *infoLabel = new QLabel(tr("Override the global azimuth offset for specific bands.\n"
                                      "Bands not listed here use the global offset."));
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    table = new QTableWidget(0, 2, this);
    table->setHorizontalHeaderLabels({tr("Band"), tr("Offset (°)")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(table);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton(tr("Add Band"));
    QPushButton *removeButton = new QPushButton(tr("Remove"));
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(dialogButtons);

    connect(addButton, &QPushButton::clicked, this, &BandOffsetDialog::addRow);
    connect(removeButton, &QPushButton::clicked, this, &BandOffsetDialog::removeRow);
    connect(dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    populateTable(offsets);
}

QStringList BandOffsetDialog::getAvailableBands() const
{
    FCT_IDENTIFICATION;

    QStringList bands;
    const QList<Band> &allBands = BandPlan::bandsList(true);

    for ( const Band &b : allBands )
        bands << b.name;

    return bands;
}

void BandOffsetDialog::populateTable(const QMap<QString, double> &offsets)
{
    FCT_IDENTIFICATION;

    QStringList bands = getAvailableBands();

    for ( auto it = offsets.constBegin(); it != offsets.constEnd(); ++it )
    {
        int row = table->rowCount();
        table->insertRow(row);

        QComboBox *bandCombo = new QComboBox();
        bandCombo->addItems(bands);
        int idx = bandCombo->findText(it.key());
        if ( idx >= 0 )
            bandCombo->setCurrentIndex(idx);
        table->setCellWidget(row, 0, bandCombo);

        QDoubleSpinBox *offsetSpin = new QDoubleSpinBox();
        offsetSpin->setRange(-180.0, 180.0);
        offsetSpin->setDecimals(1);
        offsetSpin->setSuffix("°");
        offsetSpin->setValue(it.value());
        table->setCellWidget(row, 1, offsetSpin);
    }
}

void BandOffsetDialog::addRow()
{
    FCT_IDENTIFICATION;

    QStringList bands = getAvailableBands();

    int row = table->rowCount();
    table->insertRow(row);

    QComboBox *bandCombo = new QComboBox();
    bandCombo->addItems(bands);

    // select the first band not already used
    QSet<QString> usedBands;
    for ( int i = 0; i < row; i++ )
    {
        QComboBox *existing = qobject_cast<QComboBox*>(table->cellWidget(i, 0));
        if ( existing )
            usedBands.insert(existing->currentText());
    }

    for ( int i = 0; i < bandCombo->count(); i++ )
    {
        if ( !usedBands.contains(bandCombo->itemText(i)) )
        {
            bandCombo->setCurrentIndex(i);
            break;
        }
    }

    table->setCellWidget(row, 0, bandCombo);

    QDoubleSpinBox *offsetSpin = new QDoubleSpinBox();
    offsetSpin->setRange(-180.0, 180.0);
    offsetSpin->setDecimals(1);
    offsetSpin->setSuffix("°");
    offsetSpin->setValue(0.0);
    table->setCellWidget(row, 1, offsetSpin);

    table->selectRow(row);
}

void BandOffsetDialog::removeRow()
{
    FCT_IDENTIFICATION;

    int row = table->currentRow();
    if ( row >= 0 )
        table->removeRow(row);
}

QMap<QString, double> BandOffsetDialog::getBandOffsets() const
{
    FCT_IDENTIFICATION;

    QMap<QString, double> offsets;

    for ( int row = 0; row < table->rowCount(); row++ )
    {
        QComboBox *bandCombo = qobject_cast<QComboBox*>(table->cellWidget(row, 0));
        QDoubleSpinBox *offsetSpin = qobject_cast<QDoubleSpinBox*>(table->cellWidget(row, 1));

        if ( bandCombo && offsetSpin )
        {
            QString band = bandCombo->currentText();
            if ( !band.isEmpty() )
                offsets.insert(band, offsetSpin->value());
        }
    }

    return offsets;
}
