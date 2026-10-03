#pragma once
#include <map>
#include <vector>

#include "../data/Anomaly.h"

/**
 * @file AnomalyDetector.h
 * @brief Analyzes historical temperature averages to detect acute inter-annual anomalies.
 *
 * Compares year-over-year temperature swings for identical calendar months
 * against dynamic thresholds derived from historical temperature ranges.
 */

/**
 * @brief Sequentially detects temperature anomalies across all stations.
 *
 * @param averages Nested map of monthly averages [station ID -> [year -> [month -> mean temperature]]].
 *
 * @return std::vector<Anomaly> List of detected anomalies sorted chronologically and by station.
 */
std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages);

/**
 * @brief Concurrently detects temperature anomalies across all stations.
 *
 * Accelerates anomaly detection on large datasets using std::execution::par.
 *
 * @param averages Nested map of monthly averages [station ID -> [year -> [month -> mean temperature]]].
 *
 * @return std::vector<Anomaly> List of detected anomalies sorted chronologically and by station.
 */
std::vector<Anomaly> detectAnomaliesParallel(const std::map<int, std::map<int, std::map<int, double> > > &averages);
