#include "AnomalyDetector.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>
#include <limits>
#include <ranges>
#include <tuple>
#include <iterator>

/**
 * @brief Sequentially detects temperature anomalies across all stations.
 *
 * @param averages Nested map of monthly averages [station ID -> [year -> [month -> mean temperature]]].
 *
 * @return std::vector<Anomaly> List of detected anomalies sorted chronologically and by station.
 */
std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double>>> &averages) {
    std::vector<Anomaly> result;

    for (const auto &[stationId, yearMap] : averages) {
        // Station-local min and max; indices 1-12 correspond to months
        std::array<double, 13> minVals{};
        std::array<double, 13> maxVals{};
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        // Discover global extremes for each month across the station's historical record
        for (const auto &monthMap : yearMap | std::views::values) {
            for (const auto &[month, avg] : monthMap) {
                if (avg < minVals[month])
                    minVals[month] = avg;
                if (avg > maxVals[month])
                    maxVals[month] = avg;
            }
        }

        // Precompute dynamic variance thresholds:
        // An anomaly is defined as a swing exceeding 75% of the historical temperature range
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // State holder for preceding year's observation: pair = {year, avg_temperature}
        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &fst : prevMonthData | std::views::keys)
            fst = -1; // -1 = unobserved baseline

        // Chronological anomaly evaluation
        for (const auto &[year, monthMap] : yearMap) {
            for (const auto &[month, avg] : monthMap) {
                // If years are strictly consecutive (e.g., 2021 and 2022), evaluate difference
                if (prevMonthData[month].first == year - 1) {
                    const double diff = std::abs(avg - prevMonthData[month].second);

                    if (diff > thresholds[month]) {
                        result.push_back({stationId, month, year, diff});
                    }
                }
                prevMonthData[month] = {year, avg};
            }
        }
    }

    return result;
}

/**
 * @brief Concurrently detects temperature anomalies across all stations.
 *
 * Accelerates anomaly detection on large datasets using std::execution::par.
 *
 * @param averages Nested map of monthly averages [station ID -> [year -> [month -> mean temperature]]].
 *
 * @return std::vector<Anomaly> List of detected anomalies sorted chronologically and by station.
 */
std::vector<Anomaly> detectAnomaliesParallel(const std::map<int, std::map<int, std::map<int, double>>> &averages) {
    std::vector<int> stationIds;
    stationIds.reserve(averages.size());
    for (const auto &id : averages | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Thread-isolated anomaly accumulator
    std::vector<std::vector<Anomaly>> threadResults(stationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = averages.at(stationId);
        std::vector<Anomaly> localAnomalies;

        std::array<double, 13> minVals{};
        std::array<double, 13> maxVals{};
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        for (const auto &monthMap : yearMap | std::views::values) {
            for (const auto &[month, avg] : monthMap) {
                if (avg < minVals[month])
                    minVals[month] = avg;
                if (avg > maxVals[month])
                    maxVals[month] = avg;
            }
        }

        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &p : prevMonthData | std::views::keys)
            p = -1;

        for (const auto &[year, monthMap] : yearMap) {
            for (const auto &[month, avg] : monthMap) {
                if (prevMonthData[month].first == year - 1) {
                    const double diff = std::abs(avg - prevMonthData[month].second);

                    if (diff > thresholds[month]) {
                        localAnomalies.push_back({stationId, month, year, diff});
                    }
                }
                prevMonthData[month] = {year, avg};
            }
        }

        threadResults[i] = std::move(localAnomalies);
    });

    // Sequential reduction into master list
    size_t totalAnomalies = 0;
    for (const auto &res : threadResults) {
        totalAnomalies += res.size();
    }

    std::vector<Anomaly> finalAnomalies;
    finalAnomalies.reserve(totalAnomalies);

    for (auto &res : threadResults) {
        finalAnomalies.insert(finalAnomalies.end(), std::make_move_iterator(res.begin()),
                              std::make_move_iterator(res.end()));
    }

    auto byStationYearMonth = [](const Anomaly &a, const Anomaly &b) {
        return std::tie(a.station_id, a.year, a.month) < std::tie(b.station_id, b.year, b.month);
    };

    // Thresholded sort: parallel sort overhead only amortizes above 200k items
    constexpr size_t PAR_SORT_THRESHOLD = 200000;
    if (finalAnomalies.size() >= PAR_SORT_THRESHOLD) {
        std::sort(std::execution::par, finalAnomalies.begin(), finalAnomalies.end(), byStationYearMonth);
    } else {
        std::ranges::sort(finalAnomalies, byStationYearMonth);
    }

    return finalAnomalies;
}
