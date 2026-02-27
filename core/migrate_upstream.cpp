// ============================================================
//  Upstream Migration for QLog HB9VQQ Edition
//
//  Copies data files and registry settings from upstream QLog
//  on first run. Non-destructive — upstream data is untouched.
//
//  Call migrateFromUpstreamQLog() in main() between
//  createDataDirectory() and openDatabase().
// ============================================================

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>

// ------------------------------------------------------------
//  Recursive directory copy (files only, no overwrite)
// ------------------------------------------------------------
static bool copyDirectoryRecursively(const QString &srcPath, const QString &dstPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists())
        dstDir.mkpath(".");

    // Copy files (skip if destination already exists)
    for (const QFileInfo &fi : srcDir.entryInfoList(QDir::Files))
    {
        const QString dstFile = dstPath + "/" + fi.fileName();
        if (!QFile::exists(dstFile))
        {
            if (!QFile::copy(fi.absoluteFilePath(), dstFile))
            {
                qWarning() << "Migration: failed to copy" << fi.fileName();
                // Continue — don't abort on individual file failures
            }
        }
    }

    // Recurse into subdirectories
    for (const QFileInfo &di : srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (!copyDirectoryRecursively(di.absoluteFilePath(),
                                       dstPath + "/" + di.fileName()))
            return false;
    }
    return true;
}

// ------------------------------------------------------------
//  Recursive QSettings copy (all keys and groups)
// ------------------------------------------------------------
static void copySettingsRecursively(QSettings &src, QSettings &dst)
{
    // Copy all keys in the current group
    for (const QString &key : src.childKeys())
    {
        dst.setValue(key, src.value(key));
    }

    // Recurse into subgroups
    for (const QString &group : src.childGroups())
    {
        src.beginGroup(group);
        dst.beginGroup(group);
        copySettingsRecursively(src, dst);
        dst.endGroup();
        src.endGroup();
    }
}

// ------------------------------------------------------------
//  Main migration entry point
//
//  Call AFTER setOrganizationName/setApplicationName
//  and createDataDirectory(), but BEFORE openDatabase().
// ------------------------------------------------------------
void migrateFromUpstreamQLog()
{
    // --- 1. Check if migration is needed ---

    // Edition data path (already set by setApplicationName)
    const QString editionPath = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation);

    // Upstream data path — sibling directory under same org
    const QString upstreamPath = QDir::cleanPath(editionPath + "/../QLog");

    // Guard: upstream directory must exist
    if (!QDir(upstreamPath).exists())
    {
        qInfo() << "Migration: No upstream QLog data found at" << upstreamPath;
        return;
    }

    // Guard: edition must NOT already have a database (= fresh install)
    if (QFile::exists(editionPath + "/qlog.db"))
    {
        qInfo() << "Migration: Edition already has data, skipping.";
        return;
    }

    qInfo() << "Migration: Found upstream QLog data at" << upstreamPath;
    qInfo() << "Migration: Copying to" << editionPath;

    // --- 2. Copy data directory ---

    bool filesCopied = copyDirectoryRecursively(upstreamPath, editionPath);

    // --- 3. Copy registry settings ---
    //
    // Upstream QSettings (NativeFormat = registry on Windows):
    //   HKCU\Software\hamradio\QLog
    // Edition QSettings:
    //   HKCU\Software\hamradio\QLog HB9VQQ Edition
    //
    // QSettings() default constructor uses the app's org + name,
    // so we need explicit constructors for the upstream source.

    bool settingsCopied = false;

    QSettings upstreamSettings(QSettings::NativeFormat, QSettings::UserScope,
                               "hamradio", "QLog");

    if (!upstreamSettings.childKeys().isEmpty()
        || !upstreamSettings.childGroups().isEmpty())
    {
        QSettings editionSettings;  // uses current org + app name

        // Only copy if edition settings are empty
        if (editionSettings.childKeys().isEmpty()
            && editionSettings.childGroups().isEmpty())
        {
            copySettingsRecursively(upstreamSettings, editionSettings);
            editionSettings.sync();
            settingsCopied = true;
            qInfo() << "Migration: Registry settings copied.";
        }
        else
        {
            qInfo() << "Migration: Edition already has settings, skipping.";
        }
    }

    // --- 4. Inform the user ---

    if (filesCopied || settingsCopied)
    {
        qInfo() << "Migration from upstream QLog completed.";

        QMessageBox::information(nullptr,
            QMessageBox::tr("QLog HB9VQQ Edition"),
            QMessageBox::tr(
                "Your existing QLog data has been imported.\n\n"
                "Your original QLog data remains untouched at:\n%1\n\n"
                "Registry settings have also been copied.")
            .arg(upstreamPath));
    }
    else
    {
        qWarning() << "Migration: Copy failed.";

        QMessageBox::warning(nullptr,
            QMessageBox::tr("QLog HB9VQQ Edition"),
            QMessageBox::tr(
                "Could not import your existing QLog data.\n"
                "You may need to copy files manually from:\n%1\nto:\n%2")
            .arg(upstreamPath, editionPath));
    }
}
