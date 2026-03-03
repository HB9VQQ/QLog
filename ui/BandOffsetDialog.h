#ifndef QLOG_UI_BANDOFFSETDIALOG_H
#define QLOG_UI_BANDOFFSETDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QMap>

class BandOffsetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandOffsetDialog(const QMap<QString, double> &offsets,
                              QWidget *parent = nullptr);

    QMap<QString, double> getBandOffsets() const;

private slots:
    void addRow();
    void removeRow();

private:
    void populateTable(const QMap<QString, double> &offsets);
    QStringList getAvailableBands() const;

    QTableWidget *table;
};

#endif // QLOG_UI_BANDOFFSETDIALOG_H
