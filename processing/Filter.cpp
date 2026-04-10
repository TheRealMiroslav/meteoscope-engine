#include "Filter.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <ranges>
#include <execution>

std::vector<int> filterMinYears(const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
                                const int minYears) {
    std::vector<int> result;

    int lastYear = 0;
    int counter = 0;

    for (auto &[stationId, yearMap]: groupedMeasurements) {
        for (const auto &year: yearMap | std::views::keys) {
            if (year != lastYear + 1) counter = 0;

            lastYear = year;
            counter++;

            if (counter == minYears) break;
        }

        if (counter == minYears) {
            result.push_back(stationId);
        }

        lastYear = 0;
        counter = 0;
    }

    return result;
}

std::vector<int> filterMinReadings(const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
                                   const int minPerYear) {
    std::vector<int> result;

    size_t numOfYears = 0;
    size_t numOfMeasurements = 0;

    for (auto &[stationId, yearMap]: groupedMeasurements) {
        numOfYears = yearMap.size();
        numOfMeasurements = 0;

        for (auto &[year, measurements]: yearMap) {
            numOfMeasurements += measurements.size();
        }

        const int avg = static_cast<int>(numOfMeasurements / numOfYears);
        if (avg >= minPerYear) {
            result.push_back(stationId);
        }
    }

    return result;
}

std::vector<int> filterMinYearsParallel(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minYears) {
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());

    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector passed(stationIds.size(), false);

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = groupedMeasurements.at(stationId);

        int lastYear = 0, counter = 0;

        for (const auto &year: yearMap | std::views::keys) {
            if (year != lastYear + 1) {
                counter = 0;
            }

            lastYear = year;
            counter++;

            if (counter == minYears) break;
        }

        passed[i] = (counter == minYears);
    });

    std::vector<int> result;
    for (size_t i = 0; i < stationIds.size(); i++) {
        if (passed[i]) {
            result.push_back(stationIds[i]);
        }
    }

    return result;
}

std::vector<int> filterMinReadingsParallel(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minPerYear) {
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());

    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector passed(stationIds.size(), false);

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const auto &yearMap = groupedMeasurements.at(stationIds[i]);

        size_t total = 0;

        for (const auto &ms: yearMap | std::views::values) {
            total += ms.size();
        }

        passed[i] = (static_cast<int>(total / yearMap.size()) > minPerYear);
    });

    std::vector<int> result;
    for (size_t i = 0; i < stationIds.size(); i++) {
        if (passed[i]) {
            result.push_back(stationIds[i]);
        }
    }

    return result;
}
