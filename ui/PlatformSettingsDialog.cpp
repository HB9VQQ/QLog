#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QWidget>
#include "PlatformSettingsDialog.h"
#include "ui_PlatformSettingsDialog.h"
#include "core/debug.h"
#include "ui/component/EditLine.h"

MODULE_IDENTIFICATION("qlog.ui.platformsettingsdialog");

PlatformSettingsDialog::PlatformSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlatformSettingsDialog),
    parameterCount(0)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    // Set column resize modes
    ui->settingsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->settingsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Add Continue button
    continueButton = ui->buttonBox->addButton(tr("Continue"), QDialogButtonBox::AcceptRole);
    connect(continueButton, &QPushButton::clicked, this, &QDialog::accept);
}

PlatformSettingsDialog::~PlatformSettingsDialog()
{
    delete ui;
}

void PlatformSettingsDialog::setParameters(const QList<PlatformParameter> &params,
                                           const QList<ProfileParameter> &profileParams)
{
    FCT_IDENTIFICATION;

    parameters = params;
    profilePortParameters = profileParams;
    parameterCount = params.size();

    int totalRows = params.size() + profileParams.size();
    ui->settingsTable->setRowCount(totalRows);

    int row = 0;

    // Add PlatformParameter entries
    for ( const PlatformParameter &p : params )
    {
        // Setting name
        QTableWidgetItem *nameItem = new QTableWidgetItem(p.displayName);
        ui->settingsTable->setItem(row, 0, nameItem);

        // Value - editable with browse button, imported value as placeholder
        QWidget *editWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(editWidget);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);

        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setObjectName(QString("lineEdit_%1").arg(row));
        lineEdit->setPlaceholderText(p.currentValue);
        layout->addWidget(lineEdit);

        QPushButton *browseBtn = new QPushButton("...", this);
        browseBtn->setFixedWidth(30);
        connect(browseBtn, &QPushButton::clicked, this, [this, row]() {
            browseForPath(row);
        });
        layout->addWidget(browseBtn);

        ui->settingsTable->setCellWidget(row, 1, editWidget);
        ++row;
    }

    // Add ProfilePortParameter entries
    for ( const ProfileParameter &p : profileParams )
    {
        // Setting name (includes profile name)
        QTableWidgetItem *nameItem = new QTableWidgetItem(p.displayName);
        ui->settingsTable->setItem(row, 0, nameItem);

        // Value - editable with browse button, imported value as placeholder
        QWidget *editWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(editWidget);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);

        if ( p.isExecutablePath )
        {
            QLineEdit *lineEdit = new QLineEdit(this);
            lineEdit->setObjectName(QString("lineEdit_%1").arg(row));
            lineEdit->setPlaceholderText(p.currentValue);
            layout->addWidget(lineEdit);

            QPushButton *browseBtn = new QPushButton("...",this);
            browseBtn->setFixedWidth(30);
            connect(browseBtn, &QPushButton::clicked, this, [this, row]() {
                browseForPath(row);
            });
            layout->addWidget(browseBtn);
        }
        else
        {
            SerialPortEditLine *lineEdit = new SerialPortEditLine(this);
            lineEdit->setObjectName(QString("lineEdit_%1").arg(row));
            lineEdit->setPlaceholderText(p.currentValue);
            layout->addWidget(lineEdit);
        }

        ui->settingsTable->setCellWidget(row, 1, editWidget);
        ++row;
    }
}

QList<PlatformParameter> PlatformSettingsDialog::getParameters() const
{
    FCT_IDENTIFICATION;

    QList<PlatformParameter> result = parameters;

    for ( int row = 0; row < result.size(); ++row )
    {
        QWidget *cellWidget = ui->settingsTable->cellWidget(row, 1);
        if ( cellWidget )
        {
            QLineEdit *lineEdit = cellWidget->findChild<QLineEdit*>(QString("lineEdit_%1").arg(row));
            if ( lineEdit )
            {
                qCDebug(runtime) << "new value" << lineEdit->text();
                result[row].newValue = lineEdit->text();
            }
        }
    }

    return result;
}

QList<ProfileParameter> PlatformSettingsDialog::getProfilePortParameters() const
{
    FCT_IDENTIFICATION;

    QList<ProfileParameter> result = profilePortParameters;

    for ( int i = 0; i < result.size(); ++i )
    {
        int row = parameterCount + i;
        QWidget *cellWidget = ui->settingsTable->cellWidget(row, 1);
        if ( cellWidget )
        {
            QLineEdit *lineEdit = cellWidget->findChild<QLineEdit*>(QString("lineEdit_%1").arg(row));
            if ( lineEdit )
            {
                qCDebug(runtime) << "new value" << lineEdit->text();
                result[i].newValue = lineEdit->text();
            }
        }
    }

    return result;
}

void PlatformSettingsDialog::browseForPath(int row)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << row;

    int totalRows = parameterCount + profilePortParameters.size();
    if ( row < 0 || row >= totalRows ) return;

    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Select File"),
        QString(),
        tr("All Files (*)")
    );

    if ( filename.isEmpty() ) return;

    QWidget *cellWidget = ui->settingsTable->cellWidget(row, 1);
    if ( cellWidget )
    {
        QLineEdit *lineEdit = cellWidget->findChild<QLineEdit*>(QString("lineEdit_%1").arg(row));
        if ( lineEdit )
            lineEdit->setText(filename);
    }
}
