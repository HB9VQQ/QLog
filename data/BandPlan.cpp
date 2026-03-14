#include <QSqlQuery>
#include <QSqlError>

#include "BandPlan.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.data.bandplan");

struct BandModeRange {
    double start;
    double end;
    BandPlan::BandPlanMode mode;
};

// currectly only IARU Region 1 is implemented
// https://www.iaru-r1.org/wp-content/uploads/2019/08/hf_r1_bandplan.pdf
// https://www.oevsv.at/export/shared/.content/.galleries/pdf-Downloads/OVSV-Bandplan_05-2019.pdf
static const BandModeRange r1BandModeTable[] =
{
    // 2200m
    {0.1357, 0.1378, BandPlan::BAND_MODE_CW},

    // 630m
    {0.472, 0.475, BandPlan::BAND_MODE_CW},
    {0.475, 0.479, BandPlan::BAND_MODE_DIGITAL},

    // 160m
    {1.800, 1.838, BandPlan::BAND_MODE_CW},
    {1.838, 1.840, BandPlan::BAND_MODE_DIGITAL},
    {1.840, 1.842, BandPlan::BAND_MODE_FT8},
    {1.842, 2.000, BandPlan::BAND_MODE_LSB},

    // 80m
    {3.500, 3.570, BandPlan::BAND_MODE_CW},
    {3.570, 3.573, BandPlan::BAND_MODE_DIGITAL},
    {3.573, 3.575, BandPlan::BAND_MODE_FT8},
    {3.575, 3.578, BandPlan::BAND_MODE_FT4},
    {3.578, 3.600, BandPlan::BAND_MODE_DIGITAL},
    {3.600, 4.000, BandPlan::BAND_MODE_LSB},

    // 60m
    {5.3515, 5.354, BandPlan::BAND_MODE_CW},
    {5.354, 5.500, BandPlan::BAND_MODE_LSB},

    // 40m
    {7.000, 7.040, BandPlan::BAND_MODE_CW},
    {7.040, 7.047, BandPlan::BAND_MODE_DIGITAL},
    {7.047, 7.049, BandPlan::BAND_MODE_FT4},
    {7.049, 7.060, BandPlan::BAND_MODE_DIGITAL},
    {7.060, 7.074, BandPlan::BAND_MODE_LSB},
    {7.074, 7.076, BandPlan::BAND_MODE_FT8},
    {7.076, 7.300, BandPlan::BAND_MODE_LSB},

    // 30m
    {10.100, 10.130, BandPlan::BAND_MODE_CW},
    {10.130, 10.136, BandPlan::BAND_MODE_DIGITAL},
    {10.136, 10.138, BandPlan::BAND_MODE_FT8},
    {10.138, 10.140, BandPlan::BAND_MODE_DIGITAL},
    {10.140, 10.142, BandPlan::BAND_MODE_FT4},
    {10.142, 10.150, BandPlan::BAND_MODE_DIGITAL},

    // 20m
    {14.000, 14.070, BandPlan::BAND_MODE_CW},
    {14.070, 14.074, BandPlan::BAND_MODE_DIGITAL},
    {14.074, 14.076, BandPlan::BAND_MODE_FT8},
    {14.076, 14.080, BandPlan::BAND_MODE_DIGITAL},
    {14.080, 14.082, BandPlan::BAND_MODE_FT4},
    {14.082, 14.099, BandPlan::BAND_MODE_DIGITAL},
    {14.099, 14.101, BandPlan::BAND_MODE_CW},
    {14.101, 14.350, BandPlan::BAND_MODE_USB},

    // 17m
    {18.068, 18.095, BandPlan::BAND_MODE_CW},
    {18.095, 18.100, BandPlan::BAND_MODE_DIGITAL},
    {18.100, 18.102, BandPlan::BAND_MODE_FT8},
    {18.102, 18.104, BandPlan::BAND_MODE_DIGITAL},
    {18.104, 18.106, BandPlan::BAND_MODE_FT4},
    {18.106, 18.109, BandPlan::BAND_MODE_DIGITAL},
    {18.109, 18.111, BandPlan::BAND_MODE_CW},
    {18.111, 18.168, BandPlan::BAND_MODE_USB},

    // 15m
    {21.000, 21.070, BandPlan::BAND_MODE_CW},
    {21.070, 21.074, BandPlan::BAND_MODE_DIGITAL},
    {21.074, 21.076, BandPlan::BAND_MODE_FT8},
    {21.076, 21.140, BandPlan::BAND_MODE_DIGITAL},
    {21.140, 21.142, BandPlan::BAND_MODE_FT4},
    {21.142, 21.149, BandPlan::BAND_MODE_DIGITAL},
    {21.149, 21.151, BandPlan::BAND_MODE_CW},
    {21.151, 21.450, BandPlan::BAND_MODE_USB},

    // 12m
    {24.890, 24.915, BandPlan::BAND_MODE_CW},
    {24.915, 24.917, BandPlan::BAND_MODE_FT8},
    {24.917, 24.919, BandPlan::BAND_MODE_DIGITAL},
    {24.919, 24.921, BandPlan::BAND_MODE_FT4},
    {24.921, 24.929, BandPlan::BAND_MODE_DIGITAL},
    {24.929, 24.931, BandPlan::BAND_MODE_CW},
    {24.931, 24.990, BandPlan::BAND_MODE_USB},

    // 10m
    {28.000, 28.070, BandPlan::BAND_MODE_CW},
    {28.070, 28.074, BandPlan::BAND_MODE_DIGITAL},
    {28.074, 28.076, BandPlan::BAND_MODE_FT8},
    {28.076, 28.180, BandPlan::BAND_MODE_DIGITAL},
    {28.180, 28.182, BandPlan::BAND_MODE_FT4},
    {28.182, 28.190, BandPlan::BAND_MODE_DIGITAL},
    {28.190, 28.225, BandPlan::BAND_MODE_CW},
    {28.225, 29.700, BandPlan::BAND_MODE_USB},

    // 6m
    {50.000, 50.100, BandPlan::BAND_MODE_CW},
    {50.100, 50.313, BandPlan::BAND_MODE_USB},
    {50.313, 50.315, BandPlan::BAND_MODE_FT8},
    {50.315, 50.318, BandPlan::BAND_MODE_DIGITAL},
    {50.318, 50.320, BandPlan::BAND_MODE_FT4},
    {50.320, 50.400, BandPlan::BAND_MODE_DIGITAL},
    {50.400, 50.500, BandPlan::BAND_MODE_CW},
    {50.500, 54.000, BandPlan::BAND_MODE_PHONE},

    // 4m
    {70.000, 70.100, BandPlan::BAND_MODE_CW},
    {70.100, 70.102, BandPlan::BAND_MODE_FT8},
    {70.102, 70.500, BandPlan::BAND_MODE_USB},

    // 2m
    {144.000, 144.100, BandPlan::BAND_MODE_CW},
    {144.100, 144.174, BandPlan::BAND_MODE_USB},
    {144.174, 144.176, BandPlan::BAND_MODE_FT8},
    {144.176, 144.360, BandPlan::BAND_MODE_USB},
    {144.360, 144.400, BandPlan::BAND_MODE_DIGITAL},
    {144.400, 144.491, BandPlan::BAND_MODE_CW},
    {144.491, 144.975, BandPlan::BAND_MODE_DIGITAL},
    {144.975, 148.000, BandPlan::BAND_MODE_USB},

    // 1.25m
    {222.000, 222.150, BandPlan::BAND_MODE_CW},
    {222.150, 225.000, BandPlan::BAND_MODE_USB},

    // 70cm
    {430.000, 432.000, BandPlan::BAND_MODE_USB},
    {432.000, 432.065, BandPlan::BAND_MODE_CW},
    {432.065, 432.067, BandPlan::BAND_MODE_FT8},
    {432.067, 432.100, BandPlan::BAND_MODE_CW},
    {432.100, 440.000, BandPlan::BAND_MODE_USB},

    // 33cm
    {902.000, 928.000, BandPlan::BAND_MODE_USB},

    // 23cm
    {1240.000, 1296.150, BandPlan::BAND_MODE_USB},
    {1296.150, 1296.400, BandPlan::BAND_MODE_CW},
    {1296.400, 1300.000, BandPlan::BAND_MODE_PHONE},

    // 3cm QO100
    // at the moment there is no other satellite that would be used, so we can afford it.
    // It will not affect tropo operation, because it is in the lower part of the band.
    {10489.505, 10489.540, BandPlan::BAND_MODE_CW},
    {10489.540, 10489.580, BandPlan::BAND_MODE_FT8},
    {10489.580, 10489.650, BandPlan::BAND_MODE_DIGITAL},
    {10489.650, 10489.745, BandPlan::BAND_MODE_USB},
    {10489.755, 10489.850, BandPlan::BAND_MODE_USB},
    {10489.850, 10489.990, BandPlan::BAND_MODE_PHONE}
};

static int bandModeTableSize = sizeof(r1BandModeTable) / sizeof(r1BandModeTable[0]);

BandPlan::BandPlanMode BandPlan::freq2BandMode(const double freq)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    int left = 0;
    int right = bandModeTableSize - 1;

    while ( left <= right )
    {
        int mid = (left + right) / 2;
        const BandModeRange &range = r1BandModeTable[mid];

        if (freq < range.start) right = mid - 1;
        else if (freq >= range.end) left = mid + 1;
        else return range.mode;
    }
    // fallback
    return BandPlan::BAND_MODE_PHONE;
}

const QString BandPlan::bandMode2BandModeGroupString(const BandPlanMode &bandPlanMode)
{
    FCT_IDENTIFICATION;

    switch ( bandPlanMode )
    {
    case BAND_MODE_CW: return BandPlan::MODE_GROUP_STRING_CW;

    case BAND_MODE_DIGITAL: return BandPlan::MODE_GROUP_STRING_DIGITAL;

    case BAND_MODE_FT8: return BandPlan::MODE_GROUP_STRING_FT8;

    case BAND_MODE_FT4: return BandPlan::MODE_GROUP_STRING_FT4;

    case BAND_MODE_FT2: return BandPlan::MODE_GROUP_STRING_FT2;

    case BAND_MODE_PHONE:
    case BAND_MODE_LSB:
    case BAND_MODE_USB:
        return BandPlan::MODE_GROUP_STRING_PHONE;

    case BAND_MODE_UNKNOWN:
        return QString();
    }
    return QString();
}

const QString BandPlan::freq2BandModeGroupString(const double freq)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    return bandMode2BandModeGroupString(freq2BandMode(freq));
}

const QString BandPlan::bandPlanMode2ExpectedMode(const BandPlanMode &bandPlanMode,
                                                  QString &submode)
{
    FCT_IDENTIFICATION;

    submode = QString();

    switch ( bandPlanMode )
    {
    case BAND_MODE_CW: {submode = QString(); return "CW";}
    case BAND_MODE_LSB: {submode = "LSB"; return "SSB";}
    case BAND_MODE_USB: {submode = "USB"; return "SSB";}
    case BAND_MODE_FT8: {return "FT8";}
    case BAND_MODE_FT4: {return "FT4";}
    case BAND_MODE_FT2: {return "FT2";}
    case BAND_MODE_DIGITAL: {submode = "USB"; return "SSB";} // imprecise, but let's try this
    //case BAND_MODE_PHONE: // it can be FM, SSB, AM - no Mode Change
    default:
        submode = QString();
    }

    return QString();
}

const QString BandPlan::freq2ExpectedMode(const double freq, QString &submode)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    return bandPlanMode2ExpectedMode(freq2BandMode(freq), submode);
}

const Band BandPlan::freq2Band(double freq)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    QSqlQuery query;

    if ( ! query.prepare("SELECT name, start_freq, end_freq, sat_designator "
                         "FROM bands "
                         "WHERE :freq BETWEEN start_freq AND end_freq") )
    {
        qWarning() << "Cannot prepare Select statement";
        return Band();
    }

    query.bindValue(0, freq);

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute select statement" << query.lastError();
        return Band();
    }

    if ( query.next() )
    {
        Band band;
        band.name = query.value(0).toString();
        band.start = query.value(1).toDouble();
        band.end = query.value(2).toDouble();
        band.satDesignator  = query.value(3).toString();
        return band;
    }

    return Band();
}

const Band BandPlan::bandName2Band(const QString &name)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << name;

    QSqlQuery query;

    if ( ! query.prepare("SELECT name, start_freq, end_freq, sat_designator "
                       "FROM bands "
                       "WHERE name = :name LIMIT 1") )
    {
        qWarning() << "Cannot prepare Select statement";
        return Band();
    }

    query.bindValue(0, name);

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute select statement" << query.lastError();
        return Band();
    }

    if ( query.next() )
    {
        Band band;
        band.name = query.value(0).toString();
        band.start = query.value(1).toDouble();
        band.end = query.value(2).toDouble();
        band.satDesignator  = query.value(3).toString();
        return band;
    }

    return Band();
}

const QList<Band> BandPlan::bandsList(const bool onlyDXCCBands,
                                      const bool onlyEnabled)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << onlyDXCCBands << onlyEnabled;

    QSqlQuery query;
    QList<Band> ret;

    QString stmt(QLatin1String("SELECT name, start_freq, end_freq, sat_designator "
                               "FROM bands WHERE 1 = 1 "));

    if ( onlyEnabled )
        stmt.append("AND enabled = 1 ");

    if ( onlyDXCCBands )
    {
        stmt.append("AND ((1.9 between start_freq and end_freq) "
                    "      OR (3.6 between start_freq and end_freq) "
                    "      OR (7.1 between start_freq and end_freq) "
                    "      OR (10.1 between start_freq and end_freq) "
                    "      OR (14.1 between start_freq and end_freq) "
                    "      OR (18.1 between start_freq and end_freq) "
                    "      OR (21.1 between start_freq and end_freq) "
                    "      OR (24.9 between start_freq and end_freq) "
                    "      OR (28.1 between start_freq and end_freq) "
                    "      OR (50.1 between start_freq and end_freq) "
                    "      OR (145.1 between start_freq and end_freq) "
                    "      OR (421.1 between start_freq and end_freq) "
                    "      OR (1241.0 between start_freq and end_freq) "
                    "      OR (2301.0 between start_freq and end_freq) "
                    "      OR (10001.0 between start_freq and end_freq)) ");
    }

    stmt.append("ORDER BY start_freq ");

    qCDebug(runtime) << stmt;

    if ( ! query.prepare(stmt) )
    {
        qWarning() << "Cannot prepare Select statement";
        return ret;
    }

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute select statement" << query.lastError();
        return ret;
    }

    while ( query.next() )
    {
        Band band;
        band.name = query.value(0).toString();
        band.start = query.value(1).toDouble();
        band.end = query.value(2).toDouble();
        band.satDesignator = query.value(3).toString();
        ret << band;
    }

    return ret;
}

const QString BandPlan::modeToDXCCModeGroup(const QString &mode)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << mode;

    if ( mode.isEmpty() ) return QString();

    QSqlQuery query;

    if ( !query.prepare("SELECT modes.dxcc "
                        "FROM modes "
                        "WHERE modes.name = :mode LIMIT 1"))
    {
        qWarning() << "Cannot prepare Select statement";
        return QString();
    }

    query.bindValue(0, mode);

    if ( query.exec() )
    {
        QString ret;
        query.next();
        ret = query.value(0).toString();
        return ret;
    }

    return QString();
}

const QString BandPlan::modeToModeGroup(const QString &mode)
{
    FCT_IDENTIFICATION;

    return ( mode == "FT8" ) ? BandPlan::MODE_GROUP_STRING_FT8
         : ( mode == "FT4" ) ? BandPlan::MODE_GROUP_STRING_FT4
         : ( mode == "FT2" ) ? BandPlan::MODE_GROUP_STRING_FT2
                             : BandPlan::modeToDXCCModeGroup(mode);
}

BandPlan::BandPlan()
{
    FCT_IDENTIFICATION;
}

const QString BandPlan::MODE_GROUP_STRING_CW = "CW";
const QString BandPlan::MODE_GROUP_STRING_DIGITAL = "DIGITAL";
const QString BandPlan::MODE_GROUP_STRING_FT8 = "FT8";
const QString BandPlan::MODE_GROUP_STRING_FT4 = "FT4";
const QString BandPlan::MODE_GROUP_STRING_FT2 = "FT2";
const QString BandPlan::MODE_GROUP_STRING_PHONE = "PHONE";
const QString BandPlan::MODE_GROUP_STRING_FTx = "FTx";
bool BandPlan::isFTxMode(const QString &mode) { return mode == "FT8" || mode == "FT4" || mode == "FT2"; }
bool BandPlan::isFTxBandMode(const BandPlanMode &mode) { return mode == BAND_MODE_FT8 || mode == BAND_MODE_FT4 || mode == BAND_MODE_FT2; }

/* Frequency exclusion ranges for FT8, FT4, and FT2 spots.
 * These use wider margins (±1 kHz) than the band plan table to reliably
 * catch spots even when the DX cluster node doesn't tag the mode correctly.
 * Ranges based on IARU Region 1 standard dial frequencies with margin for
 * audio offset and spot reporting variations.
 * FT2 is experimental (WSJT-X) but included for completeness.
 * See: https://github.com/foldynl/QLog/issues/937
 */

struct FreqExclusionRange {
    double start;
    double end;
};

static const FreqExclusionRange ft8ExclusionRanges[] =
{
    {1.839, 1.845},       // 160m FT8  dial 1840.0
    {3.572, 3.576},       // 80m  FT8  dial 3573.0
    {5.356, 5.362},       // 60m  FT8  dial 5357.0
    {7.073, 7.079},       // 40m  FT8  dial 7074.0
    {10.135, 10.139},     // 30m  FT8  dial 10136.0
    {14.073, 14.077},     // 20m  FT8  dial 14074.0
    {18.099, 18.103},     // 17m  FT8  dial 18100.0
    {21.073, 21.079},     // 15m  FT8  dial 21074.0
    {24.914, 24.918},     // 12m  FT8  dial 24915.0
    {28.073, 28.079},     // 10m  FT8  dial 28074.0
    {50.312, 50.316},     // 6m   FT8  dial 50313.0
};

static const FreqExclusionRange ft4ExclusionRanges[] =
{
    {3.567, 3.571},       // 80m  FT4  dial 3568.0
    {3.574, 3.578},       // 80m  FT4  dial 3575.0
    {7.046, 7.052},       // 40m  FT4  dial 7047.5
    {10.139, 10.143},     // 30m  FT4  dial 10140.0
    {14.079, 14.083},     // 20m  FT4  dial 14080.0
    {18.103, 18.107},     // 17m  FT4  dial 18104.0
    {21.139, 21.143},     // 15m  FT4  dial 21140.0
    {24.918, 24.922},     // 12m  FT4  dial 24919.0
    {28.179, 28.183},     // 10m  FT4  dial 28180.0
    {50.317, 50.321},     // 6m   FT4  dial 50318.0
};

static const FreqExclusionRange ft2ExclusionRanges[] =
{
    {1.842, 1.846},       // 160m FT2  dial 1843.0
    {3.577, 3.581},       // 80m  FT2  dial 3578.0
    {5.359, 5.363},       // 60m  FT2  dial 5360.0
    {7.051, 7.055},       // 40m  FT2  dial 7052.0
    {10.143, 10.147},     // 30m  FT2  dial 10144.0
    {14.083, 14.087},     // 20m  FT2  dial 14084.0
    {18.107, 18.111},     // 17m  FT2  dial 18108.0
    {21.143, 21.147},     // 15m  FT2  dial 21144.0
    {24.922, 24.926},     // 12m  FT2  dial 24923.0
    {28.183, 28.187},     // 10m  FT2  dial 28184.0
};

static bool isInExclusionRange(double freqMHz,
                               const FreqExclusionRange *ranges,
                               int count)
{
    for ( int i = 0; i < count; i++ )
    {
        if ( freqMHz >= ranges[i].start && freqMHz <= ranges[i].end )
            return true;
    }
    return false;
}

bool BandPlan::isInFT8FreqRange(double freqMHz)
{
    return isInExclusionRange(freqMHz, ft8ExclusionRanges,
                              sizeof(ft8ExclusionRanges) / sizeof(ft8ExclusionRanges[0]));
}

bool BandPlan::isInFT4FreqRange(double freqMHz)
{
    return isInExclusionRange(freqMHz, ft4ExclusionRanges,
                              sizeof(ft4ExclusionRanges) / sizeof(ft4ExclusionRanges[0]));
}

bool BandPlan::isInFT2FreqRange(double freqMHz)
{
    return isInExclusionRange(freqMHz, ft2ExclusionRanges,
                              sizeof(ft2ExclusionRanges) / sizeof(ft2ExclusionRanges[0]));
}
