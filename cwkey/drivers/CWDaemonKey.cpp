#include "CWDaemonKey.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.cwkey.driver.cwdaemonkey");

CWDaemonKey::CWDaemonKey(const QString &hostname,
                         const quint16 port,
                         const CWKey::CWKeyModeID mode,
                         const qint32 defaultSpeed,
                         qint32 sidetoneFrequency,
                         QObject *parent) :
    CWKey(mode, defaultSpeed, parent),
    CWKeyUDPInterface(hostname, port),
    isOpen(false),
    ESCChar(27),
    sidetoneFrequency(sidetoneFrequency)
{
    FCT_IDENTIFICATION;

    stopSendingCap = true;
    canSetKeySpeed = true;
    printKeyCaps();
}

bool CWDaemonKey::open()
{
    FCT_IDENTIFICATION;

    isOpen = isSocketReady();

    if ( isOpen && sidetoneFrequency > 0 )
    {
        QString toneCmd(ESCChar + QString("3") + QString::number(sidetoneFrequency));
        sendData(toneCmd.toLatin1());
    }

    return isOpen;
}

bool CWDaemonKey::close()
{
    FCT_IDENTIFICATION;

    isOpen = false;

    return true;
}

QString CWDaemonKey::lastError()
{
    FCT_IDENTIFICATION;
    qCDebug(runtime) << socket.error();
    qCDebug(runtime) << lastLogicalError;
    return (lastLogicalError.isEmpty()) ? socket.errorString() : lastLogicalError;
}

bool CWDaemonKey::sendText(const QString &text)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << text;

    if ( text.isEmpty() )
        return true;

    if ( !isOpen )
    {
        qCWarning(runtime) << "Key is not connected";
        lastLogicalError = tr("Keyer is not connected");
        emit keyError(tr("Cannot send Text"), lastLogicalError);
        return false;
    }

    int pos = 0;
    QRegularExpressionMatchIterator it = speedMarkerRE().globalMatch(text);
    while ( it.hasNext() )
    {
        QRegularExpressionMatch match = it.next();
        QString segment = text.mid(pos, match.capturedStart() - pos);
        segment.remove('\n');
        if ( !segment.isEmpty() )
            sendData(segment.toLatin1());

        setWPM(applySpeedMarker(match.captured(1)));

        pos = match.capturedEnd();
    }
    QString lastSegment = text.mid(pos);
    lastSegment.remove('\n');
    if ( !lastSegment.isEmpty() )
        sendData(lastSegment.toLatin1());

    return true;
}

bool CWDaemonKey::setWPM(const qint16 wpm)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << wpm;

    if ( !isOpen )
    {
        qCWarning(runtime) << "Key is not connected";
        lastLogicalError = tr("Keyer is not connected");
        emit keyError(tr("Cannot set Keyer Speed"), lastLogicalError);
        return false;
    }

    currentWPM = wpm;
    QString sentString(ESCChar + QString("2") + QString::number(wpm));
    emit keyChangedWPMSpeed(wpm);
    return (sendData(sentString.toLatin1()) > 0) ? true : false;
}

bool CWDaemonKey::immediatelyStop()
{
    FCT_IDENTIFICATION;

    if ( !isOpen )
    {
        qCWarning(runtime) << "Key is not connected";
        lastLogicalError = tr("Keyer is not connected");
        emit keyError(tr("Cannot stop Text Sending"), lastLogicalError);
        return false;
    }

    QString sentString(ESCChar + QString("4"));
    return (sendData(sentString.toLatin1()) > 0) ? true : false;
}
