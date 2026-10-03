#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "../data/Measurement.h"

/**
 * @file Aggregator.h
 * @brief Aggregates raw time-series meteorological readings into monthly statistical summaries.
 *
 * Computes mean monthly temperatures from discrete daily observations for validated stations.
 * Provides serial and parallel implementations.
 */

/**
 * @brief Sequentially computes monthly temperature averages for specified stations.
 *
 * @param groupedMeasurements Raw observations grouped by station, year, and month.
 * @param passedStationIds List of validated station IDs to process.
 *
 * @return std::map<int, std::map<int, std::map<int, double>>> Nested map of
 * calculated averages: [station ID -> [year -> [month -> mean temperature]]].
 */
std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds);

/**
 * @brief Concurrently computes monthly temperature averages for specified stations.
 *
 * High-performance multithreaded calculation distributed at station level via std::execution::par.
 * Each worker operates on station-local map accumulators (lock-free).
 *
 * @param groupedMeasurements Raw observations grouped by station, year, and month.
 * @param passedStationIds List of validated station IDs to process.
 *
 * @return std::map<int, std::map<int, std::map<int, double>>> Nested map of
 * calculated averages: [station ID -> [year -> [month -> mean temperature]]].
 */
std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAveragesParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds);
