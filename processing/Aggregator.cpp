#include "Aggregator.h"

#include <unordered_set>

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds) {
    //
    std::unordered_set<int> allowedStations(passedStationIds.begin(), passedStationIds.end());

    std::map<int, std::map<int, std::map<int, double> > > results;

    for (auto const &[stationId, yearMap]: groupedMeasurements) {
        if (allowedStations.find(stationId) == allowedStations.end()) continue;

        for (auto const &[year, measurements]: yearMap) {
            std::map<int, std::pair<double, int> > statsPerMonth;

            for (const auto &measurement: measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            for (const auto &[month, data]: statsPerMonth) {
                const double average = data.first / data.second;
                results[stationId][year][month] = average;
            }
        }
    }

    return results;
}
