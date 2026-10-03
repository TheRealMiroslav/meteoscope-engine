#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "../data/Measurement.h"

/**
 * @file Filter.h
 * @brief Functions for data hygiene and filtering meteorological stations by quality metrics.
 */

/**
 * @brief Sequentially filters stations based on minimum observation threshold criteria.
 *
 * Discards stations lacking adequate observation continuity (unbroken series of years)
 * or density (minimum average readings per active year).
 *
 * @param groupedMeasurements Hierarchical structure: station ID -> year -> measurements list.
 * @param minYears Minimum consecutive observation years required to retain station.
 * @param minPerYear Minimum average observation count per recorded year.
 *
 * @return std::vector<int> Vector of station IDs that met the quality criteria.
 */
std::vector<int> filterStationsSerial(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    int minYears,
    int minPerYear);

/**
 * @brief Concurrently filters stations based on minimum observation threshold criteria.
 *
 * Multithreaded alternative to filterStationsSerial leveraging std::execution::par
 * and lock-free thread-indexed slot assignment.
 *
 * @param groupedMeasurements Hierarchical structure: station ID -> year -> measurements list.
 * @param minYears Minimum consecutive observation years required to retain station.
 * @param minPerYear Minimum average observation count per recorded year.
 *
 * @return std::vector<int> Vector of station IDs that met the quality criteria.
 */
std::vector<int> filterStationsParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    int minYears, int minPerYear);
