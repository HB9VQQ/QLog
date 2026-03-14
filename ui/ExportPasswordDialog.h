#ifndef QLOG_UI_EXPORTPASSWORDDIALOG_H
#define QLOG_UI_EXPORTPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class ExportPasswordDialog;
}

class ExportPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportPasswordDialog(QWidget *parent = nullptr);
    ~ExportPasswordDialog();

    QString getPassword() const;
    bool getDeletePasswords() const;

private slots:
    void validatePasswords();
    void generatePassword();

private:
    Ui::ExportPasswordDialog *ui;
};

#endif // QLOG_UI_EXPORTPASSWORDDIALOG_H
