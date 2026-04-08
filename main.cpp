#include <iostream>
#include <map>
#include <windows.h>

#include "data/Station.h"
#include "io/CsvParser.h"

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

    const std::vector<Station> station = loadStations(stationPath);
    for (int i = 0; i < 10; i++) {
        std::cout << station[i].id << " " << station[i].name << std::endl;
    }


    const std::vector<Measurement> measurements = loadMeasurement(measurementsPath);

    std::map<int, std::map<int, std::vector<Measurement>>> groupedMeasurements;

    for (const auto& measurement : measurements) {
        int stationId = measurement.id;
        int year = measurement.year;

        groupedMeasurements[stationId][year].push_back(measurement);
    }

    return 0;
}
