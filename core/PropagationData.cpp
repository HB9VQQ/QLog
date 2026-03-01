#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>
#include <QSettings>

#include "PropagationData.h"
#include "debug.h"
#include "data/StationProfile.h"

#define PROPAGATION_BASE_URL "https://dxmap.hb9vqq.ch/data/qlog/"
#define PROPAGATION_POLL_INTERVAL_SEC 300

MODULE_IDENTIFICATION("qlog.core.propagationdata");

PropagationData::PropagationData(QObject *parent)
    : QObject(parent)
{
    FCT_IDENTIFICATION;

    nam = new QNetworkAccessManager(this);

    // --- Poll timer (corridors refresh) ----------------------------------
    pollTimer = new QTimer(this);
    pollTimer->setInterval(PROPAGATION_POLL_INTERVAL_SEC * 1000);
    connect(pollTimer, &QTimer::timeout, this, &PropagationData::fetchCorridors);

    // --- React to station-profile switches (grid may change) -------------
    connect(StationProfilesManager::instance(),
            &StationProfilesManager::profileChanged,
            this, &PropagationData::reloadStationProfile);

    // --- Kick off: load grid regions, then corridors ---------------------
    fetchGridRegions();

    // --- Phase 5: load ClubLog DXpedition callsigns ----------------------
    // Load cached set immediately so spots arriving before the async fetch
    // completes still get the DXpedition bypass.
    QStringList cached = QSettings().value("propagation/expeditions").toStringList();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    m_expeditionCallsigns = QSet<QString>(cached.begin(), cached.end());
#else
    m_expeditionCallsigns = QSet<QString>(QSet<QString>::fromList(cached));
#endif
    fetchExpeditions();

    // Refresh expeditions list daily
    QTimer *expTimer = new QTimer(this);
    expTimer->setInterval(24 * 3600 * 1000);
    connect(expTimer, &QTimer::timeout, this, &PropagationData::fetchExpeditions);
    expTimer->start();
}

PropagationData::~PropagationData()
{
    FCT_IDENTIFICATION;
}

PropagationData *PropagationData::instance()
{
    static PropagationData inst;
    return &inst;
}

// =========================================================================
//  Public getters
// =========================================================================

QMap<QString, PropagationData::CorridorInfo> PropagationData::corridors() const
{
    return m_corridors;
}

PropagationData::SolarInfo PropagationData::solarData() const
{
    return m_solar;
}

QDateTime PropagationData::lastUpdated() const
{
    return m_lastUpdated;
}

QString PropagationData::userTopic() const
{
    return m_userTopic;
}

QString PropagationData::userRegionName() const
{
    return m_userRegionName;
}

QString PropagationData::gridToTopic(const QString &grid2) const
{
    auto it = gridRegions.constFind(grid2.toUpper());
    return (it != gridRegions.constEnd()) ? it->topic : QString();
}

QString PropagationData::gridToName(const QString &grid2) const
{
    auto it = gridRegions.constFind(grid2.toUpper());
    return (it != gridRegions.constEnd()) ? it->name : QString();
}

bool PropagationData::isGridRegionsLoaded() const
{
    return gridRegionsLoaded;
}

bool PropagationData::isCorridorsLoaded() const
{
    return m_corridorsLoaded;
}

QMap<QString, QStringList> PropagationData::allRegionsByContinent() const
{
    QMap<QString, QSet<QString>> continentSets;

    for (auto it = gridRegions.constBegin(); it != gridRegions.constEnd(); ++it)
    {
        const GridRegionInfo &info = it.value();
        if (!info.region.isEmpty() && !info.name.isEmpty())
            continentSets[info.region].insert(info.name);
    }

    QMap<QString, QStringList> result;
    for (auto it = continentSets.constBegin(); it != continentSets.constEnd(); ++it)
    {
        QStringList sorted = it.value().values();
        sorted.sort();
        result.insert(it.key(), sorted);
    }

    return result;
}

QString PropagationData::latlonToGrid2(double lat, double lon)
{
    if (qIsNaN(lat) || qIsNaN(lon)
        || qAbs(lat) >= 90.0 || qAbs(lon) >= 180.0)
        return QString();

    QString U = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWX");
    double modLon = lon + 180.0;
    double modLat = lat + 90.0;

    int lonField = qBound(0, static_cast<int>(modLon / 20.0), 17);
    int latField = qBound(0, static_cast<int>(modLat / 10.0), 17);

    return QString(U.at(lonField)) + QString(U.at(latField));
}

QString PropagationData::userGrid2() const
{
    return m_userGrid2;
}

// =========================================================================
//  Station-profile change handler
// =========================================================================

void PropagationData::reloadStationProfile()
{
    FCT_IDENTIFICATION;

    QString oldTopic = m_userTopic;
    resolveUserRegion();

    if (m_userTopic != oldTopic && !m_userTopic.isEmpty())
    {
        qCDebug(runtime) << "User topic changed from" << oldTopic
                          << "to" << m_userTopic << "- refetching corridors";
        fetchCorridors();
    }
}

// =========================================================================
//  Grid regions (one-shot fetch at startup)
// =========================================================================

void PropagationData::fetchGridRegions()
{
    FCT_IDENTIFICATION;

    QUrl url(QString("%1grid_regions.json").arg(PROPAGATION_BASE_URL));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("QLog/%1").arg(VERSION).toUtf8());

    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() { handleGridRegionsReply(reply); });
}

void PropagationData::handleGridRegionsReply(QNetworkReply *reply)
{
    FCT_IDENTIFICATION;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "PropagationData: grid_regions.json fetch failed:"
                   << reply->errorString();
        // Retry after one poll interval
        QTimer::singleShot(PROPAGATION_POLL_INTERVAL_SEC * 1000, this,
                           &PropagationData::fetchGridRegions);
        return;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "PropagationData: grid_regions.json parse error:"
                   << parseErr.errorString();
        return;
    }

    QJsonObject gridsObj = doc.object().value("grids").toObject();
    gridRegions.clear();

    for (auto it = gridsObj.constBegin(); it != gridsObj.constEnd(); ++it)
    {
        QJsonObject g = it.value().toObject();
        GridRegionInfo info;
        info.topic  = g.value("topic").toString();
        info.region = g.value("region").toString();
        info.name   = g.value("name").toString();
        gridRegions.insert(it.key().toUpper(), info);
    }

    gridRegionsLoaded = true;
    qCDebug(runtime) << "Loaded" << gridRegions.size() << "grid regions";

    // Now resolve user region and start corridor polling
    resolveUserRegion();
    fetchCorridors();
    pollTimer->start();
}

// =========================================================================
//  User region resolution (grid → topic)
// =========================================================================

void PropagationData::resolveUserRegion()
{
    FCT_IDENTIFICATION;

    if (!gridRegionsLoaded)
        return;

    const StationProfile &profile =
        StationProfilesManager::instance()->getCurProfile1();

    QString locator = profile.locator.trimmed().toUpper();
    QString grid2 = (locator.length() >= 2) ? locator.left(2) : QString();

    if (grid2.isEmpty() || !gridRegions.contains(grid2))
    {
        qCDebug(runtime) << "Grid" << grid2
                          << "not in region map, defaulting to eu";
        m_userGrid2      = grid2;
        m_userTopic      = QStringLiteral("eu");
        m_userRegionName = QStringLiteral("Europe");
        return;
    }

    const GridRegionInfo &info = gridRegions.value(grid2);
    m_userGrid2      = grid2;
    m_userTopic      = info.topic;
    m_userRegionName = info.name;

    qCDebug(runtime) << "User grid" << grid2 << "→ topic" << m_userTopic
                      << "region" << m_userRegionName;
}

// =========================================================================
//  Corridor data (polled every 5 min)
// =========================================================================

void PropagationData::fetchCorridors()
{
    FCT_IDENTIFICATION;

    if (m_userTopic.isEmpty())
        return;

    QUrl url(QString("%1propagation_%2.json").arg(PROPAGATION_BASE_URL, m_userTopic));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("QLog/%1").arg(VERSION).toUtf8());

    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() { handleCorridorsReply(reply); });
}

void PropagationData::handleCorridorsReply(QNetworkReply *reply)
{
    FCT_IDENTIFICATION;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "PropagationData: corridor fetch failed:"
                   << reply->errorString();
        return;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "PropagationData: corridor parse error:"
                   << parseErr.errorString();
        return;
    }

    QJsonObject root = doc.object();

    // --- Solar data ------------------------------------------------------
    QJsonObject solarObj = root.value("solar").toObject();
    m_solar.kp       = solarObj.value("kp").toDouble(-1);
    m_solar.sfi      = solarObj.value("sfi").toInt(-1);
    m_solar.velocity = solarObj.value("velocity").toInt(-1);

    // --- Corridors (nested: corridors → fromGrid → target → data) --------
    QJsonObject allCorridorsObj = root.value("corridors").toObject();
    m_corridors.clear();

    // Look up corridors for user's specific grid first,
    // fall back to first available grid if user grid not present
    QJsonObject corridorsObj;
    if (allCorridorsObj.contains(m_userGrid2))
    {
        corridorsObj = allCorridorsObj.value(m_userGrid2).toObject();
    }
    else if (!allCorridorsObj.isEmpty())
    {
        corridorsObj = allCorridorsObj.begin().value().toObject();
        qCDebug(runtime) << "Grid" << m_userGrid2 << "not in corridors,"
                         << "falling back to" << allCorridorsObj.begin().key();
    }

    for (auto it = corridorsObj.constBegin(); it != corridorsObj.constEnd(); ++it)
    {
        QJsonObject c = it.value().toObject();
        CorridorInfo ci;
        ci.bestBand = c.value("best").toString();
        ci.index    = c.value("index").toInt(0);

        QJsonObject bandsObj = c.value("bands").toObject();
        for (auto b = bandsObj.constBegin(); b != bandsObj.constEnd(); ++b)
            ci.bands.insert(b.key(), b.value().toInt(0));

        m_corridors.insert(it.key(), ci);
    }

    m_lastUpdated    = QDateTime::currentDateTimeUtc();
    m_corridorsLoaded = true;

    qCDebug(runtime) << "Corridors updated:" << m_corridors.size()
                      << "targets for topic" << m_userTopic;

    emit dataUpdated();
}

// =========================================================================
//  Phase 5: ClubLog DXpedition callsigns (LoTW filter bypass)
// =========================================================================

bool PropagationData::isExpedition(const QString &callsign) const
{
    return m_expeditionCallsigns.contains(callsign.toUpper());
}

void PropagationData::fetchExpeditions()
{
    FCT_IDENTIFICATION;

    QUrl url("https://clublog.org/expeditions.php?api=1");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("QLog/%1").arg(VERSION).toUtf8());

    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() { handleExpeditionsReply(reply); });
}

void PropagationData::handleExpeditionsReply(QNetworkReply *reply)
{
    FCT_IDENTIFICATION;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "PropagationData: ClubLog expeditions fetch failed:"
                   << reply->errorString();
        // Retry after one poll interval
        QTimer::singleShot(PROPAGATION_POLL_INTERVAL_SEC * 1000, this,
                           &PropagationData::fetchExpeditions);
        return;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isArray())
    {
        qWarning() << "PropagationData: ClubLog expeditions parse error:"
                   << parseErr.errorString();
        return;
    }

    // Keep expeditions whose last QSO is within the past 12 months
    QDateTime cutoff = QDateTime::currentDateTimeUtc().addMonths(-12);
    QSet<QString> callsigns;

    QJsonArray arr = doc.array();
    for (const QJsonValue &entry : arr)
    {
        QJsonArray row = entry.toArray();
        if (row.size() < 2) continue;

        QString callsign = row.at(0).toString().toUpper();
        QString lastQsoStr = row.at(1).toString();
        QDateTime lastQso = QDateTime::fromString(lastQsoStr, "yyyy-MM-dd HH:mm:ss");
        lastQso.setTimeSpec(Qt::UTC);

        if (lastQso >= cutoff)
            callsigns.insert(callsign);
    }

    m_expeditionCallsigns = callsigns;

    // Persist for instant load on next startup
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSettings().setValue("propagation/expeditions",
                         QStringList(m_expeditionCallsigns.begin(), m_expeditionCallsigns.end()));
#else
    QSettings().setValue("propagation/expeditions",
                         m_expeditionCallsigns.toList());
#endif

    qCDebug(runtime) << "Loaded" << m_expeditionCallsigns.size()
                      << "active DXpedition callsigns from ClubLog";
}
