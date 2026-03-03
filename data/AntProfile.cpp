#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "AntProfile.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.data.antprofile");

QDataStream& operator<<(QDataStream& out, const AntProfile& v)
{

    out << v.profileName << v.description << v.azimuthBeamWidth << v.azimuthOffset << v.bandOffsets;
    return out;
}

QDataStream& operator>>(QDataStream& in, AntProfile& v)
{
    in >> v.profileName;
    in >> v.description;
    in >> v.azimuthBeamWidth;
    in >> v.azimuthOffset;

    if ( !in.atEnd() )
        in >> v.bandOffsets;

    return in;
}

double AntProfile::getEffectiveAzimuthOffset(const QString &band) const
{
    if ( !band.isEmpty() && bandOffsets.contains(band) )
        return bandOffsets.value(band);

    return azimuthOffset;
}

AntProfilesManager::AntProfilesManager() :
    ProfileManagerSQL<AntProfile>("ant_profiles")
{
    FCT_IDENTIFICATION;

    QSqlQuery profileQuery;

    if ( ! profileQuery.prepare("SELECT profile_name, desc, azimuth_beamwidth, azimuth_offset FROM ant_profiles") )
    {
        qWarning()<< "Cannot prepare select";
    }

    if ( profileQuery.exec() )
    {
        while (profileQuery.next())
        {
            AntProfile profileDB;
            profileDB.profileName = profileQuery.value(0).toString();
            profileDB.description =  profileQuery.value(1).toString();
            profileDB.azimuthBeamWidth = profileQuery.value(2).toDouble();
            profileDB.azimuthOffset = profileQuery.value(3).toDouble();

            addProfile(profileDB.profileName, profileDB);
        }
    }
    else
    {
        qInfo() << "Station Profile DB select error " << profileQuery.lastError().text();
    }

    // load per-band offsets
    QSqlQuery bandOffsetQuery;
    if ( bandOffsetQuery.prepare("SELECT profile_name, band, azimuth_offset FROM ant_band_offsets") )
    {
        if ( bandOffsetQuery.exec() )
        {
            while (bandOffsetQuery.next())
            {
                QString profName = bandOffsetQuery.value(0).toString();
                QString band = bandOffsetQuery.value(1).toString();
                double offset = bandOffsetQuery.value(2).toDouble();

                if ( profileNameList().contains(profName) )
                {
                    AntProfile prof = getProfile(profName);
                    prof.bandOffsets.insert(band, offset);
                    addProfile(profName, prof);
                }
            }
        }
        else
        {
            qCDebug(runtime) << "Band offsets table query error" << bandOffsetQuery.lastError().text();
        }
    }
    else
    {
        qCDebug(runtime) << "Band offsets table not available yet";
    }
}

void AntProfilesManager::save()
{
    FCT_IDENTIFICATION;

    QSqlQuery deleteQuery;
    QSqlQuery insertQuery;
    QSqlQuery deleteBandOffsetsQuery;
    QSqlQuery insertBandOffsetQuery;

    if ( ! deleteQuery.prepare("DELETE FROM ant_profiles") )
    {
        qWarning() << "cannot prepare Delete statement";
        return;
    }

    if ( ! insertQuery.prepare("INSERT INTO ant_profiles(profile_name, desc, azimuth_beamwidth, azimuth_offset) "
                        "VALUES (:profile_name, :desc, :azimuth_beamwidth, :azimuth_offset)") )
    {
        qWarning() << "cannot prepare Insert statement";
        return;
    }

    if ( ! deleteBandOffsetsQuery.prepare("DELETE FROM ant_band_offsets") )
    {
        qWarning() << "cannot prepare Delete band offsets statement";
        return;
    }

    if ( ! insertBandOffsetQuery.prepare("INSERT INTO ant_band_offsets(profile_name, band, azimuth_offset) "
                                         "VALUES (:profile_name, :band, :azimuth_offset)") )
    {
        qWarning() << "cannot prepare Insert band offset statement";
        return;
    }

    if ( deleteQuery.exec() )
    {
        deleteBandOffsetsQuery.exec();

        const QStringList &keys = profileNameList();
        for ( const QString &key: keys )
        {
            const AntProfile &antProfile = getProfile(key);

            insertQuery.bindValue(":profile_name", key);
            insertQuery.bindValue(":desc", antProfile.description);
            insertQuery.bindValue(":azimuth_beamwidth", antProfile.azimuthBeamWidth);
            insertQuery.bindValue(":azimuth_offset", antProfile.azimuthOffset);


            if ( ! insertQuery.exec() )
            {
                qInfo() << "Station Profile DB insert error " << insertQuery.lastError().text() << insertQuery.lastQuery();
            }

            // save per-band offsets
            for ( auto it = antProfile.bandOffsets.constBegin(); it != antProfile.bandOffsets.constEnd(); ++it )
            {
                insertBandOffsetQuery.bindValue(":profile_name", key);
                insertBandOffsetQuery.bindValue(":band", it.key());
                insertBandOffsetQuery.bindValue(":azimuth_offset", it.value());

                if ( ! insertBandOffsetQuery.exec() )
                {
                    qInfo() << "Band offset DB insert error " << insertBandOffsetQuery.lastError().text();
                }
            }
        }
    }
    else
    {
        qInfo() << "Station Profile DB delete error " << deleteQuery.lastError().text();
    }

    saveCurProfile1();

}

bool AntProfile::operator==(const AntProfile &profile)
{
    return (profile.profileName == this->profileName
            && profile.description == this->description
            && profile.azimuthBeamWidth == this->azimuthBeamWidth
            && profile.azimuthOffset == this->azimuthOffset
            && profile.bandOffsets == this->bandOffsets);
}

bool AntProfile::operator!=(const AntProfile &profile)
{
   return !operator==(profile);
}
