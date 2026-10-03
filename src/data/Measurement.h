#pragma once

/**
 * @file Measurement.h
 *
 * @brief Data structure representing a single raw meteorological measurement.
 *
 * Holds raw data ingested from CSV input streams prior to aggregation and filtering.
 */
struct Measurement {
    int id;         ///< Identifier of the observing station.
    int ordinal;    ///< Ordinal sequence number (e.g., day of year or absolute record index).
    int year;       ///< Observation calendar year.
    int month;      ///< Observation month (1-12).
    float value;    ///< Observed metric value (e.g., mean daily temperature in Celsius).
};