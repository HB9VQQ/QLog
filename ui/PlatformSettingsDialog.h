#ifndef QLOG_UI_PLATFORMSETTINGSDIALOG_H
#define QLOG_UI_PLATFORMSETTINGSDIALOG_H

#include <QDialog>
#include <QPushButton>
#include "core/PlatformParameterManager.h"

namespace Ui {
class PlatformSettingsDialog;
}

class PlatformSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlatformSettingsDialog(QWidget *parent = nullptr);
    ~PlatformSettingsDialog();

    void setParameters(const QList<PlatformParameter> &params,
                       const QList<ProfileParameter> &profileParams);
    QList<PlatformParameter> getParameters() const;
    QList<ProfileParameter> getProfilePortParameters() const;

private slots:
    void browseForPath(int row);

private:
    Ui::PlatformSettingsDialog *ui;
    QPushButton *continueButton;
    QList<PlatformParameter> parameters;
    QList<ProfileParameter> profilePortParameters;
    int parameterCount;  // Number of PlatformParameter rows
};

#endif // QLOG_UI_PLATFORMSETTINGSDIALOG_H
