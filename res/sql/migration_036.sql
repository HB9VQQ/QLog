CREATE TABLE IF NOT EXISTS ant_band_offsets (
    profile_name TEXT NOT NULL,
    band TEXT NOT NULL,
    azimuth_offset DOUBLE DEFAULT 0.0,
    PRIMARY KEY(profile_name, band)
);
