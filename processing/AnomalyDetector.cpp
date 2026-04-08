#include "AnomalyDetector.h"

#include <algorithm>

std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double>>> &averages) {
    std::vector<Anomaly> result;

    std::map<int, std::map<int, std::vector<std::pair<int, double>>>> monthlyHistory;

    for (auto const &[stationId, yearMap] : averages) {
        for (auto const &[year, monthMap] : yearMap) {
            for (auto const &[month, avg] : monthMap) {
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
