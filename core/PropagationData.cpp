#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>

#include "PropagationData.h"
#include "debug.h"
#include "data/StationProfile.h"

MODULE_IDENTIFICATION("qlog.core.propagationdata");

PropagationData::PropagationData(QObject *parent)
    : QObject(parent)
{
    FCT_IDENTIFICATION;

    nam = new QNetworkAccessManager(this);

    // --- Poll timer (corridors refresh) ----------------------------------
    pollTimer = new QTimer(this);
    pollTimer->setInterval(POLL_INTERVAL_SEC * 1000);
    connect(pollTimer, &QTimer::timeout, this, &PropagationData::fetchCorridors);

    // --- React to station-profile switches (grid may change) -------------
    connect(StationProfilesManager::instance(),
            &StationProfilesManager::profileChanged,
            this, &PropagationData::reloadStationProfile);

    // --- Kick off: load grid regions, then corridors ---------------------
    fetchGridRegions();
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
        qCDebug(function) << "User topic changed from" << oldTopic
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

    QUrl url(QString("%1grid_regions.json").arg(BASE_URL));
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
        QTimer::singleShot(POLL_INTERVAL_SEC * 1000, this,
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
    qCDebug(function) << "Loaded" << gridRegions.size() << "grid regions";

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
        qCDebug(function) << "Grid" << grid2
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

    qCDebug(function) << "User grid" << grid2 << "→ topic" << m_userTopic
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

    QUrl url(QString("%1propagation_%2.json").arg(BASE_URL, m_userTopic));
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

    // --- Corridors -------------------------------------------------------
    QJsonObject corridorsObj = root.value("corridors").toObject();
    m_corridors.clear();

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

    qCDebug(function) << "Corridors updated:" << m_corridors.size()
                      << "targets for topic" << m_userTopic;

    emit dataUpdated();
}
