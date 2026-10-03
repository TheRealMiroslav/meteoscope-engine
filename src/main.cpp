/**
 * @file main.cpp
 *
 * @brief Main entry point for the MeteoScope meteorological analysis engine.
 *
 * Handles CLI argument validation, console UTF-8 initialization,
 * serial or parallel dataset ingestion, anomaly detection, statistical aggregation,
 * and vector (SVG) and tabular (CSV) output generation.
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

#include "data/Station.h"
#include "io/CsvParser.h"
#include "core/AppRunner.h"
#include "utils/Timer.h"

/**
 * @brief Application entry point.
 *
 * Expects exactly 3 command-line arguments:
 * 1. Path to stations CSV file
 * 2. Path to measurements CSV file
 * 3. Processing mode flag (--serial or --parallel)
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of argument strings.
 * @return 0 on success, 1 on argument or runtime error.
 */
int main(const int argc, char const *argv[]) {
#ifdef _WIN32
    // Set console code page to UTF-8 for international characters
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "MeteoScope Engine - High-Performance Meteorological Analysis Engine\n";
        std::cout << "Usage: " << argv[0] << " <stations.csv> <measurements.csv> <--serial|--parallel>\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help    Show this help message and exit\n";
        std::cout << "  --serial      Execute single-threaded analysis\n";
        std::cout << "  --parallel    Execute multi-threaded analysis (Parallel STL / thread pools)\n";
        return 0;
    }

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <stations.csv> <measurements.csv> <--serial|--parallel>\n";
        std::cerr << "Try '" << argv[0] << " --help' for more information.\n";
        return 1;
    }

    const std::string stationPath = argv[1];
    const std::string measurementsPath = argv[2];
    const std::string mode = argv[3];

    if (mode != "--serial" && mode != "--parallel") {
        std::cerr << "Error: Invalid mode flag '" << mode << "'. Use --serial or --parallel.\n";
        return 1;
    }

    std::vector<Station> stations;
    std::vector<Measurement> measurements;

    // --- TIMED EXECUTION BLOCK ---
    Timer totalTimer;

    // Phase 1: Ingestion
    Timer loadTimer;
    if (mode == "--serial") {
        stations = loadStationsSerial(stationPath);
        measurements = loadMeasurementSerial(measurementsPath);
    } else {
        stations = loadStationsParallel(stationPath);
        measurements = loadMeasurementParallel(measurementsPath);
    }
    loadTimer.stop();

    if (stations.empty()) {
        std::cerr << "Error: Failed to load stations from '" << stationPath << "'. File does not exist or is empty.\n";
        return 1;
    }

    if (measurements.empty()) {
        std::cerr << "Error: Failed to load measurements from '" << measurementsPath
                  << "'. File does not exist or is empty.\n";
        return 1;
    }

    // Phase 2: Processing (filtering, monthly aggregation, anomaly detection, SVG generation)
    Timer processTimer;
    if (mode == "--serial") {
        runSerial(stations, measurements);
    } else {
        runParallel(stations, measurements);
    }
    processTimer.stop();

    totalTimer.stop();
    // --- END TIMED BLOCK ---

    std::cout << "================================================\n";
    std::cout << "       METEOROLOGICAL ANALYSIS COMPLETE         \n";
    std::cout << "================================================\n";
    std::cout << " Mode:              " << (mode == "--parallel" ? "PARALLEL" : "SERIAL") << "\n";
    std::cout << " Station Count:     " << stations.size() << "\n";
    std::cout << " Measurement Count: " << measurements.size() << "\n";
    std::cout << "------------------------------------------------\n";

    std::cout << std::fixed << std::setprecision(3);

    std::cout << " Load Time:         " << loadTimer.elapsedSeconds() << " s\n";
    std::cout << " Processing Time:   " << processTimer.elapsedSeconds() << " s\n";
    std::cout << "------------------------------------------------\n";
    std::cout << " TOTAL TIME:        " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << "================================================\n";

    return 0;
}
