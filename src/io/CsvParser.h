#pragma once
#include <vector>
#include <string>

#include "../data/Measurement.h"
#include "../data/Station.h"

/**
 * @file CsvParser.h
 *
 * @brief High-throughput zero-copy CSV parsing module.
 *
 * Provides serial and parallel implementations for ingesting station metadata
 * and large time-series measurement streams with optimal memory buffering and CPU utilization.
 */

/**
 * @brief Ingests meteorological station records from a CSV file (sequentially).
 *
 * @param path Filesystem path to the stations CSV.
 *
 * @return std::vector<Station> Vector of parsed stations. Returns empty vector on failure.
 */
std::vector<Station> loadStationsSerial(const std::string &path);

/**
 * @brief Ingests time-series measurements from a CSV file (sequentially).
 *
 * @param path Filesystem path to the measurements CSV.
 *
 * @return std::vector<Measurement> Vector of parsed measurements. Returns empty vector on failure.
 */
std::vector<Measurement> loadMeasurementSerial(const std::string &path);

/**
 * @brief Concurrently ingests time-series measurements from a CSV file across worker threads.
 *
 * Splits the memory-buffered file into newline-aligned chunks processed concurrently
 * using std::thread::hardware_concurrency(), accelerating ingestion of massive datasets.
 *
 * @param path Filesystem path to the measurements CSV.
 *
 * @return std::vector<Measurement> Vector of parsed measurements. Returns empty vector on failure.
 */
std::vector<Measurement> loadMeasurementParallel(const std::string &path);

/**
 * @brief Concurrently ingests meteorological station records from a CSV file across worker threads.
 *
 * @param path Filesystem path to the stations CSV.
 *
 * @return std::vector<Station> Vector of parsed stations. Returns empty vector on failure.
 */
std::vector<Station> loadStationsParallel(const std::string &path);
