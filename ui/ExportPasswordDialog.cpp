#include <QPushButton>
#include <QRandomGenerator>
#include "ExportPasswordDialog.h"
#include "ui_ExportPasswordDialog.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.exportpassworddialog");

ExportPasswordDialog::ExportPasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportPasswordDialog)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ExportPasswordDialog::~ExportPasswordDialog()
{
    FCT_IDENTIFICATION;

    delete ui;
}

QString ExportPasswordDialog::getPassword() const
{
    FCT_IDENTIFICATION;
    return ui->passwordEdit->text();
}

bool ExportPasswordDialog::getDeletePasswords() const
{
    FCT_IDENTIFICATION;
    return ui->deletePasswordsCheckBox->isChecked();
}

void ExportPasswordDialog::validatePasswords()
{
    FCT_IDENTIFICATION;

    const QString &pass = ui->passwordEdit->text();
    const QString &confirm = ui->confirmEdit->text();
    if ( pass.isEmpty() )
    {
        ui->confirmEdit->clear();
        ui->passwordEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        ui->confirmEdit->setEchoMode(QLineEdit::Password);
    }

    bool isEnabled = (!pass.isEmpty() && pass == confirm);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isEnabled);
}

void ExportPasswordDialog::generatePassword()
{
    FCT_IDENTIFICATION;

    // removing similar chars 0 and Oo, 1 and lI
    static const char charset[] = "aBcDeFgHiJkLmNPqRsTuVwXyZ"
                                  "AbCdEfGhjKMnOpQrStUvWxYz"
                                  "@2$4&6!78#93%5*";

    const int charsetLen = static_cast<int>(sizeof(charset) - 1);
    const int passwordLen = 12;

    QRandomGenerator *rng = QRandomGenerator::system();
    QString password;
    password.reserve(passwordLen);

    for ( int i = 0; i < passwordLen; ++i )
        password.append(QChar(charset[rng->bounded(charsetLen)]));

    ui->passwordEdit->setText(password);
    ui->confirmEdit->setText(password);

    ui->passwordEdit->setEchoMode(QLineEdit::Normal);
    ui->confirmEdit->setEchoMode(QLineEdit::Normal);
}
