// Stubs for missing dependencies in CredentialStore tests
// These provide minimal implementations to satisfy linker

#include <QString>
#include "core/LogDatabase.h"
#include "data/BandPlan.h"

// Stub for LogDatabase::currentPlatformId()
QString LogDatabase::currentPlatformId()
{
    return QStringLiteral("TestPlatform");
}

// Stubs for BandPlan constants used in LogParam
const QString BandPlan::MODE_GROUP_STRING_PHONE = QStringLiteral("PHONE");
const QString BandPlan::MODE_GROUP_STRING_CW = QStringLiteral("CW");
const QString BandPlan::MODE_GROUP_STRING_FTx = QStringLiteral("FTx");
const QString BandPlan::MODE_GROUP_STRING_DIGITAL = QStringLiteral("DIGITAL");
