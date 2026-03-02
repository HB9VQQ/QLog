# QLog HB9VQQ Edition

**A fork of [QLog](https://github.com/foldynl/QLog) by OK1MLG with FT8/FT4/FT2 frequency exclusion, real-time propagation awareness, and DXpedition-safe LoTW filtering.**

This edition adds features for the serious DXer — reliable mode filtering, propagation-driven spot scoring, and smart LoTW filtering that won't hide active DXpeditions.

## What's Different

**FT8/FT4/FT2 Frequency Exclusion Filter** — a two-layer filtering approach ([upstream issue #937](https://github.com/foldynl/QLog/issues/937)):

- **Layer 1 — Independent mode groups:** FT4 and FT2 get their own checkboxes in the DX filter dialog (previously FT4 was lumped into Digital)
- **Layer 2 — Frequency-range exclusion:** When FT8/FT4/FT2 checkboxes are unchecked, spots are also rejected by frequency range (±1 kHz around standard WSJT-X dial frequencies). This catches spots where the cluster node tagged the mode incorrectly or left it blank

**Real-Time Propagation Awareness** — powered by DX Index data from HB9VQQ:

- **Propagation Window** — dockable widget showing best band per target region from your QTH, updated every 5 minutes
- **DX Table Columns** — Spotter Region, Score (0–100 workability index), and Distance columns in the DX cluster spot list
- **Propagation Filter Tab** — filter spots by minimum workability score, spotter sub-region, and sort by score. Sub-region filter supersedes the coarser Spotter Continent filter with automatic mutual exclusion

**DXpedition-Safe LoTW Filtering** — automatically bypasses the LoTW member filter for active DXpeditions:

- Fetches the [ClubLog Expeditions list](https://clublog.org/expeditions.php) at startup and refreshes daily
- DXpedition callsigns (last QSO within 12 months) always pass through the LoTW filter
- Enables safe use of Member → LoTW filtering without losing spots like 3Y0K or KP5/NP3VI that upload to LoTW only after the expedition ends
- Works for both DX cluster spots and WSJT-X alerts

**Per-Band Antenna Bearing Offset** — correct rotor commands for antennas with different radiation patterns per band:

- Extends the antenna profile (Settings → Equipment → Antennas) with per-band azimuth offsets
- Solves the Spiderbeam Yagi + 40m add-on dipole problem: the dipole radiates broadside (perpendicular to boom) while Yagi elements radiate end-fire, requiring a −90° offset on 40m
- When QLog sends a rotor command, it applies the per-band offset if defined, otherwise the global offset
- Equivalent to PstRotatorAz's "ANT 2 BD-90" feature, built into QLog


## Download

**Windows builds** are available on the [Releases](https://github.com/HB9VQQ/QLog/releases) page:

- **Installer** (recommended) — run `qlog-installer.exe`
- **Portable ZIP** — extract anywhere and run `qlog.exe`

## Side-by-Side with Upstream QLog

This edition installs and runs independently from upstream QLog:

| | Upstream QLog | HB9VQQ Edition |
|---|---|---|
| Install directory | `Program Files\QLog` | `Program Files\QLog HB9VQQ Edition` |
| Data directory | `AppData\Local\hamradio\QLog` | `AppData\Local\hamradio\QLog HB9VQQ Edition` |
| Registry | `HKCU\Software\hamradio\QLog` | `HKCU\Software\hamradio\QLog HB9VQQ Edition` |
| Start Menu | QLog | QLog HB9VQQ Edition |

Both versions can be installed and run at the same time.

## Upgrading from Upstream QLog

On first launch, the HB9VQQ Edition automatically detects and copies your existing QLog data (database, settings, backups). Your original upstream data is never modified — it's a non-destructive copy. A dialog confirms the migration.

## Staying in Sync

This fork tracks upstream releases. When OK1MLG publishes updates, they are merged in and a new build is published here.

## Support

This is a personal project. I don't have the time or resources to test every possible combination of hardware, operating system, and software configuration. Support is provided on a **best-effort basis only** — I may look into issues when time permits, but there is no guarantee of a fix or response. No user is entitled to support of any kind.

If something doesn't work, you're welcome to open an [issue](https://github.com/HB9VQQ/QLog/issues), but please understand that this fork is provided as-is. For issues related to core QLog functionality (not the features listed above), please report them to the [upstream project](https://github.com/foldynl/QLog/issues) instead.

*73 de Roland, HB9VQQ*

