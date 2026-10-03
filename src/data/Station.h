#pragma once

/**
 * @file Station.h
 *
 * @brief Represents a meteorological observation station and its geographic location.
 *
 * Used for mapping raw time-series data to physical geospatial coordinates,
 * required for SVG vector cartography.
 */
struct Station {
    int id;         ///< Unique station identifier.
    double lat;     ///< Latitude in decimal degrees.
    double lon;     ///< Longitude in decimal degrees.
};