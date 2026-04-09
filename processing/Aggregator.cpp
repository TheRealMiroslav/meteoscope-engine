#include "Aggregator.h"

#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <execution>

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds) {
    //
    std::unordered_set<int> allowedStations(passedStationIds.begin(), passedStationIds.end());

    std::map<int, std::map<int, std::map<int, double> > > results;

    for (auto const &[stationId, yearMap]: groupedMeasurements) {
        if (!allowedStations.contains(stationId)) continue;

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

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAveragesParallel(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds) {

    std::vector<int> indices(passedStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    using StationResult = std::map<int, std::map<int, double>>;
    std::vector<StationResult> threadResult(passedStationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](int i) {
        const int stationId = passedStationIds.at(i);

        const auto &yearMap = groupedMeasurements.at(stationId);

        StationResult localStationResult;

        for (auto const &[year, measurements]: yearMap) {
            std::map<int, std::pair<double, int> > statsPerMonth;

            for (const auto &measurement: measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            for (const auto &[month, data]: statsPerMonth) {
                const double average = data.first / data.second;
                localStationResult[year][month] = average;
            }
        }
        threadResult[i] = localStationResult;
    });

    std::map<int, std::map<int, std::map<int, double> > > finalResults;
    for (size_t i = 0; i < passedStationIds.size(); i++) {
        int stationId = passedStationIds.at(i);
        finalResults[stationId] = std::move(threadResult.at(i));
    }

    return finalResults;
}
