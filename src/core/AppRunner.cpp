#include "AppRunner.h"

#include <map>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <limits>
#include <ranges>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "../processing/Filter.h"
#include "../processing/Aggregator.h"
#include "../processing/AnomalyDetector.h"
#include "../io/CsvWriter.h"
#include "../output/SvgWriter.h"
#include "../utils/Config.h"

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
void runSerial(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    // 1. Data Grouping
    // Structure: station ID -> (year -> measurements vector)
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(stations.size());

    for (const auto &measurement: measurements) {
        groupedMeasurements[measurement.id][measurement.year].push_back(measurement);
    }

    // 2. Station Filtering
    // Filter stations lacking at least 5 consecutive years and 100 observations/year
    std::vector<int> passedFilters = filterStationsSerial(groupedMeasurements, 5, 100);

    // 3. Average Calculation
    // Aggregate observations into monthly means for validated stations
    auto monthlyAverages = computeMonthlyAverages(groupedMeasurements, passedFilters);

    // 4. Extremes Discovery
    // Identify absolute dataset bounds for normalization of heatmap colors
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    for (const auto &yearMap: monthlyAverages | std::views::values) {
        for (const auto &monthMap: yearMap | std::views::values) {
            for (const auto &avg: monthMap | std::views::values) {
                if (avg < globalMin) globalMin = avg;
                if (avg > globalMax) globalMax = avg;
            }
        }
    }

    // 5. Anomaly Detection and Reporting
    const std::vector<Anomaly> anomalies = detectAnomalies(monthlyAverages);
    writeSerialAnomaliesCsv(anomalies, Config::OUTPUT_SERIAL_FLUCTUATION_DIR);

    // 6. Station Map Filtering & SVG Generation
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());
    std::vector<Station> filteredStations;

    for (const auto &s: stations) {
        if (passedSet.contains(s.id)) {
            filteredStations.push_back(s);
        }
    }

    writeSvgMapsSerial(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                       Config::OUTPUT_SERIAL_MAPS_DIR);
}

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
void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    // 1. Data Grouping (Parallel Map Phase)
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

    // Each thread maintains its isolated unordered_map partition to avoid mutex lock contention
    std::vector<std::unordered_map<int, std::map<int, std::vector<Measurement> > > > localMaps(nThreads);

    std::vector<std::thread> threads;
    const size_t chunkSize = (measurements.size() + nThreads - 1) / nThreads;

    for (size_t t = 0; t < nThreads; t++) {
        threads.emplace_back([&, t]() {
            const size_t start = t * chunkSize;
            const size_t end = std::min(start + chunkSize, measurements.size());

            for (size_t i = start; i < end; i++) {
                const auto &m = measurements[i];
                localMaps[t][m.id][m.year].push_back(m);
            }
        });
    }

    for (auto &thread: threads) thread.join();

    // Extract all distinct station IDs across all thread partitions
    std::unordered_set<int> allStationIdsSet;
    for (const auto &localMap: localMaps) {
        for (const auto &sid: localMap | std::views::keys) {
            allStationIdsSet.insert(sid);
        }
    }

    std::vector<int> allStationIds(allStationIdsSet.begin(), allStationIdsSet.end());
    std::vector<std::map<int, std::vector<Measurement> > > mergedVec(allStationIds.size());
    std::vector<size_t> mergeIndices(allStationIds.size());
    std::iota(mergeIndices.begin(), mergeIndices.end(), 0);

    // Reduction Phase: parallel aggregation across station indices
    std::for_each(std::execution::par, mergeIndices.begin(), mergeIndices.end(), [&](size_t i) {
        const int sid = allStationIds[i];

        for (const auto &localMap: localMaps) {
            const auto it = localMap.find(sid);
            if (it == localMap.end()) continue;

            for (const auto &[year, ms]: it->second) {
                auto &target = mergedVec[i][year];
                target.insert(target.end(), ms.begin(), ms.end());
            }
        }
    });

    // Assemble final master map via move semantics
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(allStationIds.size());

    for (size_t i = 0; i < allStationIds.size(); i++) {
        if (!mergedVec[i].empty()) {
            groupedMeasurements[allStationIds[i]] = std::move(mergedVec[i]);
        }
    }

    // 2. Filtering Phase
    std::vector<int> passedFilters = filterStationsParallel(groupedMeasurements, 5, 100);

    // 3. Averages Calculation Phase
    auto monthlyAverages = computeMonthlyAveragesParallel(groupedMeasurements, passedFilters);

    // 4. Extremes Discovery Phase
    std::vector<int> activeStationIds;
    activeStationIds.reserve(monthlyAverages.size());
    for (const auto &sid: monthlyAverages | std::views::keys) activeStationIds.push_back(sid);

    // Lock-free local extremes collector per station index
    std::vector<std::pair<double, double> > localExtremes(activeStationIds.size(), {
                                                              std::numeric_limits<double>::max(),
                                                              std::numeric_limits<double>::lowest()
                                                          });

    std::vector<size_t> indices(activeStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        double lMin = std::numeric_limits<double>::max();
        double lMax = std::numeric_limits<double>::lowest();

        for (const auto &monthMap: monthlyAverages.at(activeStationIds[i]) | std::views::values) {
            for (const auto &avg: monthMap | std::views::values) {
                if (avg < lMin) lMin = avg;
                if (avg > lMax) lMax = avg;
            }
        }

        localExtremes[i] = {lMin, lMax};
    });

    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    for (const auto &[fst, snd]: localExtremes) {
        if (fst < globalMin) globalMin = fst;
        if (snd > globalMax) globalMax = snd;
    }

    // 5. Anomaly Detection and Reporting
    const std::vector<Anomaly> anomalies = detectAnomaliesParallel(monthlyAverages);
    writeParallelAnomaliesCsv(anomalies, Config::OUTPUT_PARALLEL_FLUCTUATION_DIR);

    // 6. Station Map Filtering & SVG Generation
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());

    std::vector<Station> filteredStations;
    filteredStations.resize(stations.size());

    auto it = std::copy_if(std::execution::par, stations.begin(), stations.end(), filteredStations.begin(),
                           [&](const Station &s) { return passedSet.contains(s.id); });

    filteredStations.erase(it, filteredStations.end());

    if (!std::filesystem::exists(Config::OUTPUT_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_DIR);
    }

    if (!std::filesystem::exists(Config::OUTPUT_PARALLEL_MAPS_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_PARALLEL_MAPS_DIR);
    }

    writeSvgMapsParallel(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                         Config::OUTPUT_PARALLEL_MAPS_DIR);
}
