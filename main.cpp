#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <windows.h>

#include "data/Station.h"
#include "io/CsvParser.h"
#include "io/CsvWriter.h"
#include "output/SvgWriter.h"
#include "processing/Aggregator.h"
#include "processing/AnomalyDetector.h"
#include "processing/Filter.h"

int main(int argc, char const *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc <= 3 || argc >= 5) {
        std::cout << "Wrong number of arguments\n";
        return -1;
    }

    const std::string stationPath = argv[1];
    const std::string measurementsPath = argv[2];
    std::string mode = argv[3];

    const std::vector<Station> stations = loadStations(stationPath);
    const std::vector<Measurement> measurements = loadMeasurement(measurementsPath);

    // první klič = ID stanice
    // druhý klíč = Rok
    std::map<int, std::map<int, std::vector<Measurement>>> groupedMeasurements;

    for (const auto& measurement : measurements) {
        int stationId = measurement.id;
        int year = measurement.year;

        groupedMeasurements[stationId][year].push_back(measurement);
    }

    std::vector<int> passedFirstFilter = filterMinYears(groupedMeasurements, 5);
    std::vector<int> passedSecondFilter = filterMinReadings(groupedMeasurements, 100);

    std::sort(passedFirstFilter.begin(), passedFirstFilter.end());
    std::sort(passedSecondFilter.begin(), passedSecondFilter.end());

    std::vector<int> passedFilters = {};
    std::set_intersection(passedFirstFilter.begin(), passedFirstFilter.end(), passedSecondFilter.begin(), passedSecondFilter.end(), std::back_inserter(passedFilters));

    const std::map<int, std::map<int, std::map<int, double>>> monthlyAverages = computeMonthlyAverages(groupedMeasurements, passedFilters);

    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    for (auto const& [stationId, yearMap] : monthlyAverages) {
        for (auto const& [year, monthMap] : yearMap) {
            for (auto const& [month, avg] : monthMap) {
                if (avg < globalMin) globalMin = avg;
                if (avg > globalMax) globalMax = avg;
            }
        }
    }

    std::vector<Anomaly> anomalies = detectAnomalies(monthlyAverages);

    writeAnomaliesCsv(anomalies, "./");

    std::vector<Station> filteredStations;
    for (const auto& s : stations) {
        if (std::find(passedFilters.begin(), passedFilters.end(), s.id) != passedFilters.end()) {
            filteredStations.push_back(s);
        }
    }

    writeSvgMaps(filteredStations, monthlyAverages, globalMin, globalMax, "./", "./");

    return 0;
}
