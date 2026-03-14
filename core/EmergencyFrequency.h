#ifndef QLOG_CORE_EMERGENCYFREQUENCY_H
#define QLOG_CORE_EMERGENCYFREQUENCY_H

#include <QString>

struct EmergencyFreqEntry
{
    double  frequency;    // MHz
    QString mode;
};

class EmergencyFrequency
{
public:
    static double TOLERANCE_MHZ;
    static const QList<EmergencyFreqEntry> &list();
    static const EmergencyFreqEntry *inBand(double startMHz, double endMHz);
    static const EmergencyFreqEntry *findEmergency(double freqMHz);
};

#endif // QLOG_CORE_EMERGENCYFREQUENCY_H
