#include "Aggregator.h"

#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <execution>
#include <array>
#include <unordered_map>

/**
 * @brief Sequentially computes monthly temperature averages for specified stations.
 *
 * @param groupedMeasurements Raw observations grouped by station, year, and month.
 * @param passedStationIds List of validated station IDs to process.
 *
 * @return std::map<int, std::map<int, std::map<int, double>>> Nested map of
 * calculated averages: [station ID -> [year -> [month -> mean temperature]]].
 */
std::map<int, std::map<int, std::map<int, double>>>
computeMonthlyAverages(const std::unordered_map<int, std::map<int, std::vector<Measurement>>> &groupedMeasurements,
                       const std::vector<int> &passedStationIds) {
    const std::unordered_set<int> allowedStations(passedStationIds.begin(), passedStationIds.end());

    std::map<int, std::map<int, std::map<int, double>>> results;

    for (auto const &[stationId, yearMap] : groupedMeasurements) {
        if (!allowedStations.contains(stationId))
            continue;

        for (auto const &[year, measurements] : yearMap) {
            // Indices 1-12 correspond to months (index 0 unused)
            // first = sum of values, second = count of readings
            std::array<std::pair<double, int>, 13> statsPerMonth{};

            for (const auto &measurement : measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            for (int month = 1; month <= 12; ++month) {
                if (statsPerMonth[month].second > 0) {
                    const double average = statsPerMonth[month].first / statsPerMonth[month].second;
                    results[stationId][year][month] = average;
                }
            }
        }
    }

    return results;
}

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
std::map<int, std::map<int, std::map<int, double>>> computeMonthlyAveragesParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement>>> &groupedMeasurements,
    const std::vector<int> &passedStationIds) {
    std::vector<int> indices(passedStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    using StationResult = std::map<int, std::map<int, double>>;
    std::vector<StationResult> threadResult(passedStationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = passedStationIds.at(i);
        const auto &yearMap = groupedMeasurements.at(stationId);

        StationResult localStationResult;

        for (auto const &[year, measurements] : yearMap) {
            std::array<std::pair<double, int>, 13> statsPerMonth{};

            for (const auto &measurement : measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            for (int month = 1; month <= 12; ++month) {
                if (statsPerMonth[month].second > 0) {
                    const double average = statsPerMonth[month].first / statsPerMonth[month].second;
                    localStationResult[year][month] = average;
                }
            }
        }
        threadResult[i] = std::move(localStationResult);
    });

    // Sequential reduction into master map
    std::map<int, std::map<int, std::map<int, double>>> finalResults;
    for (size_t i = 0; i < passedStationIds.size(); i++) {
        finalResults[passedStationIds[i]] = std::move(threadResult[i]);
    }

    return finalResults;
}
