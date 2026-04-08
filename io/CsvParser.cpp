#include "CsvParser.h"
#include <fstream>
#include <sstream>

std::vector<Station> loadStations(const std::string& path) {
    std::vector<Station> result;
    std::ifstream file(path);

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        Station station;

        std::getline(ss, token, ';');
        station.id = std::stoi(token);

        std::getline(ss, token, ';');
        station.name = token;

        std::getline(ss, token, ';');
        station.lat = std::stod(token);

        std::getline(ss, token, ';');
        station.lon = std::stod(token);

        result.push_back(station);
    }

    return result;
}

std::vector<Measurement> loadMeasurement(const std::string& path) {
    std::vector<Measurement> result;
    std::ifstream file(path);

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        Measurement measurement;

        std::getline(ss, token, ';');
        measurement.id = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.ordinal = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.year = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.month = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.value = std::stof(token);

        result.push_back(measurement);
    }

    return result;
}

