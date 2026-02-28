#ifndef QLOG_CORE_PROPAGATIONDATA_H
#define QLOG_CORE_PROPAGATIONDATA_H

#include <QObject>
#include <QMap>
#include <QDateTime>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

/**
 * @brief Singleton providing real-time propagation data from dx.hb9vqq.ch.
 *
 * Phase 1 foundation — shared by Phases 2 and 3 without modification.
 *
 * Startup: fetches grid_regions.json once (grid → topic + region name).
 * Timer:   polls propagation_{topic}.json every 5 min.
 * Emits:   dataUpdated() after every successful corridor parse.
 */
class PropagationData : public QObject
{
    Q_OBJECT

public:
    struct GridRegionInfo
    {
        QString topic;   // "eu", "na", "sa", "as", "oc", "af", "pac"
        QString region;  // "Europe", "North America", …
        QString name;    // "Central Europe", "US East Coast", …
    };

    struct CorridorInfo
    {
        QString bestBand;                // "10m", "15m", "20m", …
        int     index = 0;               // 0–100, best-band DX Index
        QMap<QString, int> bands;        // band → score
    };

    struct SolarInfo
    {
        double kp       = -1;
        int    sfi      = -1;
        int    velocity = -1;
    };

    static PropagationData *instance();

    // --- Corridor data (refreshed every 5 min) --------------------------
    QMap<QString, CorridorInfo> corridors() const;
    SolarInfo  solarData()      const;
    QDateTime  lastUpdated()    const;

    // --- User region (derived from station profile grid) -----------------
    QString userTopic()         const;
    QString userRegionName()    const;

    // --- Grid ↔ region helpers (Phase 2 spotter resolution) --------------
    QString gridToTopic(const QString &grid2) const;
    QString gridToName(const QString &grid2)  const;

    // --- DXCC → grid helper (Phase 2: lat/lon → 2-letter Maidenhead) -----
    static QString latlonToGrid2(double lat, double lon);
    QString userGrid2() const;

    bool isGridRegionsLoaded() const;
    bool isCorridorsLoaded()   const;

signals:
    void dataUpdated();

public slots:
    void reloadStationProfile();

private:
    explicit PropagationData(QObject *parent = nullptr);
    ~PropagationData() override;

    // Network
    QNetworkAccessManager *nam;
    QTimer *pollTimer;

    // Grid regions (loaded once at startup)
    QMap<QString, GridRegionInfo> gridRegions;  // key = 2-letter grid (e.g. "JN")
    bool gridRegionsLoaded = false;

    // User context
    QString m_userTopic;       // "eu" etc.
    QString m_userRegionName;  // "Central Europe" etc.
    QString m_userGrid2;       // "JN" etc.

    // Corridor data (refreshed every poll)
    QMap<QString, CorridorInfo> m_corridors;
    SolarInfo  m_solar;
    QDateTime  m_lastUpdated;
    bool       m_corridorsLoaded = false;

    // Internal
    static constexpr int POLL_INTERVAL_SEC = 300;   // 5 min
    static constexpr const char *BASE_URL =
        "https://dxmap.hb9vqq.ch/data/qlog/";

    void fetchGridRegions();
    void fetchCorridors();
    void resolveUserRegion();

private slots:
    void handleGridRegionsReply(QNetworkReply *reply);
    void handleCorridorsReply(QNetworkReply *reply);
};

#endif // QLOG_CORE_PROPAGATIONDATA_H
