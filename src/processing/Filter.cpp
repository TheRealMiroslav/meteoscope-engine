#include "Filter.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <ranges>
#include <execution>
#include <unordered_map>
#include <numeric>

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
    const int minYears,
    const int minPerYear) {
    std::vector<int> result;
    // Heuristic capacity pre-allocation (~75% retention rate)
    result.reserve((groupedMeasurements.size() / 4) * 3);

    for (const auto &[stationId, yearMap]: groupedMeasurements) {
        if (yearMap.empty())
            continue;

        // Step 1: Measurement density check
        size_t totalMeasurements = 0;
        for (const auto &ms: yearMap | std::views::values) {
            totalMeasurements += ms.size();
        }

        if (static_cast<int>(totalMeasurements / yearMap.size()) < minPerYear)
            continue;

        // Step 2: Temporal continuity check (consecutive years)
        int lastYear = 0;
        int counter = 0;
        bool passedYears = false;

        // Iteration over std::map keys is inherently chronological (ascending)
        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passedYears = true;
                break;
            }
        }

        if (passedYears) {
            result.push_back(stationId);
        }
    }

    return result;
}

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
    const int minYears,
    const int minPerYear) {
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());
    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    // Indicator array for lock-free recording
    std::vector<int> passed(stationIds.size(), 0);
    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Concurrent quality evaluation per station
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = groupedMeasurements.at(stationId);

        if (yearMap.empty()) return;

        // Step 1: Measurement density check
        size_t totalMeasurements = 0;
        for (const auto &ms: yearMap | std::views::values) {
            totalMeasurements += ms.size();
        }

        if (static_cast<int>(totalMeasurements / yearMap.size()) < minPerYear) {
            return;
        }

        // Step 2: Temporal continuity check
        int lastYear = 0;
        int counter = 0;
        bool passedYears = false;

        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passedYears = true;
                break;
            }
        }

        if (passedYears) {
            passed[i] = 1; // Thread writes only to its dedicated slot (lock-free)
        }
    });

    // Sequential reduction: collect IDs that passed criteria
    std::vector<int> result;
    result.reserve((stationIds.size() / 4) * 3);

    for (size_t i = 0; i < stationIds.size(); i++) {
        if (passed[i]) {
            result.push_back(stationIds[i]);
        }
    }

    return result;
}
