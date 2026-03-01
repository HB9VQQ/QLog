#ifndef QLOG_UI_DXFILTER_H
#define QLOG_UI_DXFILTER_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class DxFilterDialog;
}

class DxFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DxFilterDialog(QWidget *parent = nullptr);
    ~DxFilterDialog();
    void accept() override;

private:
    Ui::DxFilterDialog *ui;
    QList<QCheckBox*> memberListCheckBoxes;
    QList<QCheckBox*> regionCheckBoxes;
    const quint8 MAXCOLUMNS = 6;
    const quint8 REGION_MAXCOLUMNS = 4;
    void generateMembershipCheckboxes();
    void generateRegionCheckboxes();

};

#endif // QLOG_UI_DXFILTER_H
