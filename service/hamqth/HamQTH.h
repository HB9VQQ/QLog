#ifndef QLOG_SERVICE_HAMQTH_HAMQTH_H
#define QLOG_SERVICE_HAMQTH_HAMQTH_H

#include <QObject>
#include <QString>
#include "service/GenericCallbook.h"
#include "core/CredentialStore.h"

class QNetworkAccessManager;
class QNetworkReply;

class HamQTHBase : public SecureServiceBase<HamQTHBase>
{
protected:
    const static QString SECURE_STORAGE_KEY;

public:
    explicit HamQTHBase() {};
    virtual ~HamQTHBase() {};

    DECLARE_SECURE_SERVICE(HamQTHBase);

    static const QString getUsername();
    static const QString getPasswd();
    static void saveUsernamePassword(const QString&, const QString&);
};

class HamQTHCallbook : public GenericCallbook, private HamQTHBase
{
    Q_OBJECT

public:
    const static QString CALLBOOK_NAME;

    explicit HamQTHCallbook(QObject *parent = nullptr);
    virtual ~HamQTHCallbook();

    QString getDisplayName() override;

public slots:
    virtual void queryCallsign(const QString &callsign) override;
    virtual void abortQuery() override;

protected:
    void processReply(QNetworkReply* reply) override;

private:
    const QString API_URL = "https://www.hamqth.com/xml.php";
    QString sessionId;
    QString queuedCallsign;
    bool incorrectLogin;
    QString lastSeenPassword;
    QNetworkReply *currentReply;

    void authenticate();
};

#endif // QLOG_SERVICE_HAMQTH_HAMQTH_H
