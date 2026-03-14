#ifndef QLOG_UI_LOADDATABASEDIALOG_H
#define QLOG_UI_LOADDATABASEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QSqlDatabase>
#include "core/LogDatabase.h"

namespace Ui {
class LoadDatabaseDialog;
}

class LoadDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadDatabaseDialog(QWidget *parent = nullptr);
    ~LoadDatabaseDialog();

    QString getSelectedFile() const;
    QString getDecompressedFile() const;
    QString takeDecompressedFile();  // Returns path and transfers ownership (caller must delete)
    QString getPassword() const;
    bool isCrossPlatform() const;
    bool requiresPassword() const;

public slots:
    void accept() override;

private slots:
    void browseFile();
    void updateLoadButtonState();

private:
    void openImportDatabase();
    void closeImportDatabase();
    bool verifyPassword();

    Ui::LoadDatabaseDialog *ui;
    QPushButton *loadButton;

    QString selectedFile;
    DatabaseInfo dbInfo;

    // Cached encrypted passwords blob from import DB
    QByteArray encryptedPasswordsBlob;

    // Temporary decompressed file path
    QString tempDecompressedFile;

    static const QString importConnectionName;
};

#endif // QLOG_UI_LOADDATABASEDIALOG_H
