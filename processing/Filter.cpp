#include "Filter.h"

#include <map>
#include <ranges>

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
        if (avg > minPerYear) {
            result.push_back(stationId);
        }
    }

    return result;
}
