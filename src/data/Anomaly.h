#pragma once

/**
 * @file Anomaly.h
 *
 * @brief Represents a detected meteorological anomaly.
 *
 * Stores metadata on where and when a significant inter-annual temperature swing occurred,
 * along with the magnitude of deviation.
 */
struct Anomaly {
    int station_id; ///< Identifier of the station where the anomaly occurred.
    int month;      ///< Month of the anomaly event (1-12).
    int year;       ///< Year of the anomaly event.
    double diff;    ///< Magnitude of temperature deviation relative to previous year.
};
