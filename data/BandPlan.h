#ifndef QLOG_DATA_BANDPLAN_H
#define QLOG_DATA_BANDPLAN_H

#include <QtCore>
#include "Band.h"

class BandPlan
{
public:

    static const QString MODE_GROUP_STRING_CW;
    static const QString MODE_GROUP_STRING_DIGITAL;
    static const QString MODE_GROUP_STRING_FT8;
    static const QString MODE_GROUP_STRING_FT4;
    static const QString MODE_GROUP_STRING_FT2;
    static const QString MODE_GROUP_STRING_PHONE;
    static const QString MODE_GROUP_STRING_FTx;
    static bool isFTxMode(const QString &mode);
    static bool isFTxBandMode(const BandPlanMode &mode);

    enum BandPlanMode
    {
        BAND_MODE_UNKNOWN,
        BAND_MODE_CW,
        BAND_MODE_DIGITAL,
        BAND_MODE_FT8,
        BAND_MODE_FT4,
        BAND_MODE_FT2,
        BAND_MODE_LSB,
        BAND_MODE_USB,
        BAND_MODE_PHONE
    };

    static BandPlanMode freq2BandMode(const double freq);
    static const QString bandMode2BandModeGroupString(const BandPlan::BandPlanMode &bandPlanMode);
    static const QString freq2BandModeGroupString(const double freq);
    static const QString bandPlanMode2ExpectedMode(const BandPlan::BandPlanMode &bandPlanMode,
                                                   QString &submode);
    static const QString freq2ExpectedMode(const double freq,
                                     QString &submode);
    static const Band freq2Band(double freq);
    static const Band bandName2Band(const QString& name);
    static const QList<Band> bandsList(const bool onlyDXCCBands = false,
                                       const bool onlyEnabled = false);
    static const QString modeToDXCCModeGroup(const QString &mode);
    static const QString modeToModeGroup(const QString &mode);
    static bool isInFT8FreqRange(double freqMHz);
    static bool isInFT4FreqRange(double freqMHz);
    static bool isInFT2FreqRange(double freqMHz);
    BandPlan();
};

Q_DECLARE_METATYPE(BandPlan::BandPlanMode);

#endif // QLOG_DATA_BANDPLAN_H
