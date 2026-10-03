#include "CsvWriter.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <numeric>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Serial export of detected meteorological anomalies to CSV format.
 *
 * @param anomalies Vector of detected anomalies to export.
 * @param filePath Output file destination path.
 *
 * @throws std::runtime_error If the target file cannot be opened for writing.
 */
void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    fs::path path(filePath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Failed to create anomalies CSV file: " + filePath);
    }

    file << "station_id;month;year;diff\n";

    for (const auto &[station_id, month, year, diff] : anomalies) {
        file << station_id << ";" << month << ";" << year << ";" << diff << "\n";
    }

    file.close();
}

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
void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    fs::path path(filePath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Failed to create anomalies CSV file: " + filePath);
    }

    // 1. Write CSV header (sequentially)
    file << "station_id;month;year;diff\n";

    if (anomalies.empty()) {
        return;
    }

    // 2. Pre-allocate line string slots
    std::vector<std::string> lines(anomalies.size());
    std::vector<size_t> indices(anomalies.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 3. Parallel string formatting across worker threads
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const auto &[station_id, month, year, diff] = anomalies[i];

        char buffer[128];
        std::snprintf(buffer, sizeof(buffer), "%d;%d;%d;%g\n", station_id, month, year, diff);
        lines[i] = std::string(buffer);
    });

    // 4. Sequential flush to disk to preserve atomic file structure
    for (const auto &line : lines) {
        file << line;
    }

    file.close();
}
