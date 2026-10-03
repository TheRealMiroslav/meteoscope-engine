#pragma once
#include <string>
#include <vector>

#include "../data/Anomaly.h"

/**
 * @file CsvWriter.h
 * @brief Utilities for serializing anomaly detection results into CSV files.
 */

/**
 * @brief Serial export of detected meteorological anomalies to CSV format.
 *
 * @param anomalies Vector of detected anomalies to export.
 * @param filePath Output file destination path.
 *
 * @throws std::runtime_error If the target file cannot be opened for writing.
 */
void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);

/**
 * @brief Parallelized export of detected meteorological anomalies to CSV format.
 *
 * Decouples CPU-bound text formatting (multithreaded via std::execution::par)
 * from sequential disk I/O to avoid filesystem race conditions.
 *
 * @param anomalies Vector of detected anomalies to export.
 * @param filePath Output file destination path.
 *
 * @throws std::runtime_error If the target file cannot be opened for writing.
 */
void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);
