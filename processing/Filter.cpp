#include "Filter.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <ranges>
#include <execution>
#include <unordered_map>
#include <numeric>

std::vector<int> filterMinYears(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minYears) {
    std::vector<int> result;
    result.reserve(groupedMeasurements.size());

    for (const auto &[stationId, yearMap]: groupedMeasurements) {
        int lastYear = 0;
        int counter = 0;
        bool passed = false;

        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passed = true;
                break;
            }
        }

        if (passed) {
            result.push_back(stationId);
        }
    }

    return result;
}

std::vector<int> filterMinReadings(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minPerYear) {
    std::vector<int> result;
    result.reserve(groupedMeasurements.size());

    for (const auto &[stationId, yearMap]: groupedMeasurements) {
        size_t numOfYears = yearMap.size();
        size_t numOfMeasurements = 0;

        for (const auto &measurements: yearMap | std::views::values) {
            numOfMeasurements += measurements.size();
        }

        if (numOfYears > 0) {
            const int avg = static_cast<int>(numOfMeasurements / numOfYears);
            if (avg >= minPerYear) {
                result.push_back(stationId);
            }
        }
    }

    return result;
}

std::vector<int> filterMinYearsParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minYears) {
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());

    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector<int> passed(stationIds.size(), 0);
    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = groupedMeasurements.at(stationId);

        int lastYear = 0, counter = 0;

        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passed[i] = 1;
                break;
            }
        }
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
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minPerYear) {
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());

    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector<int> passed(stationIds.size(), 0);
    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const auto &yearMap = groupedMeasurements.at(stationIds[i]);
        size_t total = 0;

        for (const auto &ms: yearMap | std::views::values) {
            total += ms.size();
        }

        if (!yearMap.empty() && static_cast<int>(total / yearMap.size()) >= minPerYear) {
            passed[i] = 1;
        }
    });

    std::vector<int> result;
    for (size_t i = 0; i < stationIds.size(); i++) {
        if (passed[i]) {
            result.push_back(stationIds[i]);
        }
    }

    return result;
}