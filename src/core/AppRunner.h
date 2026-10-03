#pragma once

#include <vector>

#include "../data/Station.h"
#include "../data/Measurement.h"

/**
 * @file AppRunner.h
 * @brief Orchestrates end-to-end meteorological data processing pipelines.
 */

/**
 * @brief Serial execution pipeline for meteorological data processing.
 *
 * Sequentially groups raw data, filters stations by historical continuity and density,
 * computes monthly averages, discovers global extremes, detects anomalies,
 * and writes CSV reports and SVG vector maps in a single thread.
 *
 * @param stations Vector of all ingested meteorological stations.
 * @param measurements Vector of all ingested time-series measurements.
 */
void runSerial(const std::vector<Station> &stations, const std::vector<Measurement> &measurements);

/**
 * @brief Parallel execution pipeline for high-throughput meteorological data processing.
 *
 * Executes the identical computational stages as runSerial, but leverages multi-threading
 * (std::thread, std::execution::par) and lock-free thread-local Map-Reduce reduction patterns
 * to scale across all available CPU cores.
 *
 * @param stations Vector of all ingested meteorological stations.
 * @param measurements Vector of all ingested time-series measurements.
 */
void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements);
