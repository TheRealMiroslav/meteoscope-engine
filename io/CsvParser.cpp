#include "CsvParser.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <sstream>
#include <charconv>
#include <iostream>
#include <system_error>


std::vector<Station> loadStations(const std::string &path) {
    std::vector<Station> result;
    std::ifstream file(path);

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        Station station{};

        std::getline(ss, token, ';');
        station.id = std::stoi(token);

        // name skip
        std::getline(ss, token, ';');
        //station.name = token;

        std::getline(ss, token, ';');
        station.lat = std::stod(token);

        std::getline(ss, token, ';');
        station.lon = std::stod(token);

        result.push_back(station);
    }

    return result;
}

std::vector<Measurement> loadMeasurement(const std::string &path) {
    std::vector<Measurement> result;
    std::ifstream file(path);

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        Measurement measurement{};

        std::getline(ss, token, ';');
        measurement.id = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.ordinal = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.year = std::stoi(token);

        std::getline(ss, token, ';');
        measurement.month = std::stoi(token);

        // day skip
        std::getline(ss, token, ';');

        //value
        std::getline(ss, token, ';');
        std::ranges::replace(token, ',', '.');

        if (!token.empty() && token.back() == '\r') {
            token.pop_back();
        }

        measurement.value = std::stof(token);

        result.push_back(measurement);
    }

    return result;
}

std::vector<Station> loadStationsParallel(const std::string &path) {
    std::vector<Station> result;
    std::ifstream file(path);

    if (!file.is_open()) return result;

    std::string line;
    std::getline(file, line); // Hlavička

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Station station{};
        const char *p = line.data();
        const char *end = p + line.size();

        auto [ptr1, ec1] = std::from_chars(p, end, station.id);
        p = ptr1 + 1;

        while (p < end && *p != ';') ++p;
        ++p;

        auto [ptr2, ec2] = std::from_chars(p, end, station.id);
        p = ptr2 + 1;

        std::from_chars(p, end, station.id);

        result.push_back(station);
    }

    return result;
}

std::vector<Measurement> loadMeasurementParallel(const std::string &path) {
    std::vector<Measurement> result;
    std::ifstream file(path);

    if (!file.is_open()) return result;

    std::string line;
    std::getline(file, line); // Hlavička

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Measurement m{};
        const char *p = line.data();
        const char *end = p + line.size();

        // 1. id
        auto [ptr1, ec1] = std::from_chars(p, end, m.id);
        p = ptr1 + 1; // přeskočíme ';'

        // 2. ordinal
        auto [ptr2, ec2] = std::from_chars(p, end, m.ordinal);
        p = ptr2 + 1;

        // 3. year
        auto [ptr3, ec3] = std::from_chars(p, end, m.year);
        p = ptr3 + 1;

        // 4. month
        auto [ptr4, ec4] = std::from_chars(p, end, m.month);
        p = ptr4 + 1;

        // 5. day (přeskočíme)
        while (p < end && *p != ';') ++p;
        ++p; // přeskočíme ';'

        // 6. value (ošetření desetinné čárky a \r z Windows)
        char valBuf[32];
        size_t len = 0;
        while (p < end && *p != '\r' && *p != '\n' && len < 31) {
            valBuf[len++] = (*p == ',') ? '.' : *p;
            ++p;
        }
        std::from_chars(valBuf, valBuf + len, m.value);

        result.push_back(m);
    }
    return result;
}

