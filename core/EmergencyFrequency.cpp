#include "EmergencyFrequency.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.core.emergencyfrequency");

double EmergencyFrequency::TOLERANCE_MHZ = 0.001;

const QList<EmergencyFreqEntry> &EmergencyFrequency::list()
{
    FCT_IDENTIFICATION;

    static const QList<EmergencyFreqEntry> freqs =
    {
        {   3.760, "LSB"},
        {   7.110, "LSB"},
        {  14.300, "USB"},
        {  18.160, "USB"},
        {  21.360, "USB"},
        { 145.550, "FM"},
        { 433.550, "FM"},
    };
    return freqs;
}

const EmergencyFreqEntry *EmergencyFrequency::inBand(double startMHz, double endMHz)
{
    FCT_IDENTIFICATION;

    for ( const EmergencyFreqEntry &entry : list() )
        if ( entry.frequency >= startMHz && entry.frequency <= endMHz )
            return &entry;

    return nullptr;
}

const EmergencyFreqEntry *EmergencyFrequency::findEmergency(double freqMHz)
{
    FCT_IDENTIFICATION;

    const QList<EmergencyFreqEntry> &freqs = EmergencyFrequency::list();

    for ( const EmergencyFreqEntry &entry : freqs )
        if ( qAbs(freqMHz - entry.frequency) <= EmergencyFrequency::TOLERANCE_MHZ )
            return &entry;

    return nullptr;
}
