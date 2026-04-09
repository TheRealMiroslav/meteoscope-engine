#include "AnomalyDetector.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>

std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages) {
    std::vector<Anomaly> result;

    std::map<int, std::map<int, std::vector<std::pair<int, double> > > > monthlyHistory;

    for (auto const &[stationId, yearMap]: averages) {
        for (auto const &[year, monthMap]: yearMap) {
            for (auto const &[month, avg]: monthMap) {
                monthlyHistory[stationId][month].emplace_back(year, avg);
            }
        }
    }

    for (auto &[stationId, monthMap]: monthlyHistory) {
        for (auto &[month, dataPoints]: monthMap) {
            std::ranges::sort(dataPoints);

            if (dataPoints.empty()) continue;

            double minVal = dataPoints[0].second;
            double maxVal = dataPoints[0].second;
            for (const auto &dataPoint: dataPoints) {
                if (dataPoint.second < minVal) minVal = dataPoint.second;
                if (dataPoint.second > maxVal) maxVal = dataPoint.second;
            }

            const double threshold = 0.75 * (maxVal - minVal);

            for (size_t i = 0; i < dataPoints.size() - 1; ++i) {
                const int year1 = dataPoints[i].first;
                const int year2 = dataPoints[i + 1].first;

                const double val1 = dataPoints[i].second;
                const double val2 = dataPoints[i + 1].second;

                if (year2 == year1 + 1) {
                    const double diff = std::abs(val2 - val1);

                    if (diff > threshold) {
                        result.push_back({stationId, month, year2, diff});
                    }
                }
            }
        }
    }

    return result;
}

std::vector<Anomaly> detectAnomaliesParallel(
    const std::map<int, std::map<int, std::map<int, double>>> &averages) {

    std::vector<int> stationIds;
    stationIds.reserve(averages.size());
    for (auto const &[id, _] : averages)
        stationIds.push_back(id);

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::vector<Anomaly>> threadResults(stationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = stationIds[i];

        std::array<std::vector<std::pair<int, double>>, 13> monthData;

        for (const auto &[year, monthMap] : averages.at(stationId))
            for (const auto &[month, avg] : monthMap)
                monthData[month].emplace_back(year, avg);

        std::vector<Anomaly> localAnomalies;

        for (int month = 1; month <= 12; month++) {
            const auto &dataPoints = monthData[month];
            if (dataPoints.size() < 2) continue;

            double minVal = dataPoints[0].second;
            double maxVal = dataPoints[0].second;
            for (const auto &[year, val] : dataPoints) {
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }

            const double threshold = 0.75 * (maxVal - minVal);

            for (size_t j = 0; j < dataPoints.size() - 1; ++j) {
                const int year1 = dataPoints[j].first;
                const int year2 = dataPoints[j + 1].first;

                if (year2 != year1 + 1) continue;

                const double diff = std::abs(dataPoints[j + 1].second - dataPoints[j].second);
                if (diff > threshold)
                    localAnomalies.push_back({stationId, month, year2, diff});
            }
        }

        threadResults[i] = std::move(localAnomalies);
    });

    std::vector<Anomaly> finalAnomalies;
    for (const auto &res : threadResults)
        finalAnomalies.insert(finalAnomalies.end(), res.begin(), res.end());

    return finalAnomalies;
}
