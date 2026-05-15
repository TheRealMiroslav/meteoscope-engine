#include "CsvParser.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <sstream>
#include <charconv>
#include <iostream>
#include <system_error>
#include <thread>


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

std::vector<Station> loadStationsOptimized(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};

    std::vector<Station> result;
    result.reserve(size / 50);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n') lineEnd++;

        if (lineEnd > p) {
            Station station{};
            const char *curr = p;

            // ID
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, station.id);
            curr = ptr1;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Name (přeskočíme)
            while (curr < lineEnd && *curr != ';') ++curr;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Lat
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, station.lat);
            curr = ptr2;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Lon
            std::from_chars(curr, lineEnd, station.lon);

            result.push_back(station);
        }
        p = lineEnd + 1;
    }

    return result;
}

std::vector<Measurement> loadMeasurementOptimized(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};

    std::vector<Measurement> result;
    result.reserve(size / 30);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n') lineEnd++;

        if (lineEnd > p) {
            Measurement m{};
            const char *curr = p;

            // id
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, m.id);
            curr = ptr1 + 1; // přeskočíme ';'

            // ordinal
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, m.ordinal);
            curr = ptr2 + 1;

            // year
            auto [ptr3, ec3] = std::from_chars(curr, lineEnd, m.year);
            curr = ptr3 + 1;

            // month
            auto [ptr4, ec4] = std::from_chars(curr, lineEnd, m.month);
            curr = ptr4 + 1;

            // day (přeskočíme)
            while (curr < lineEnd && *curr != ';') ++curr;
            ++curr;

            // value
            char valBuf[32];
            size_t len = 0;
            while (curr < lineEnd && *curr != '\r' && len < 31) {
                valBuf[len++] = (*curr == ',') ? '.' : *curr;
                ++curr;
            }
            std::from_chars(valBuf, valBuf + len, m.value);

            result.push_back(m);
        }
        p = lineEnd + 1;
    }
    return result;
}

std::vector<Measurement> loadMeasurementParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};
    size_t startPos = headerEnd + 1;

    // Výpočet hranic pro jednotlivá vlákna
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *> > chunks;

    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        // Zarovnání endPos na konec řádku, abychom nepřerušili data v půlce
        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n') {
                endPos++;
            }
            if (endPos < size) endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    // Paralelní parsování v chunkách
    std::vector<std::vector<Measurement> > localResults(chunks.size());
    std::vector<std::thread> threads;

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            // Hrubý odhad kapacity (cca 30 znaků/řádek)
            localVec.reserve((end - p) / 30);

            while (p < end) {
                const char *lineEnd = p;
                // Najdeme konec aktuálního řádku
                while (lineEnd < end && *lineEnd != '\n') lineEnd++;

                if (lineEnd > p) {
                    Measurement m{};
                    const char *curr = p;

                    // id
                    auto [ptr1, ec1] = std::from_chars(curr, lineEnd, m.id);
                    curr = ptr1 + 1; // přeskočíme ';'

                    // ordinal
                    auto [ptr2, ec2] = std::from_chars(curr, lineEnd, m.ordinal);
                    curr = ptr2 + 1;

                    // year
                    auto [ptr3, ec3] = std::from_chars(curr, lineEnd, m.year);
                    curr = ptr3 + 1;

                    // month
                    auto [ptr4, ec4] = std::from_chars(curr, lineEnd, m.month);
                    curr = ptr4 + 1;

                    // day (přeskočíme)
                    while (curr < lineEnd && *curr != ';') ++curr;
                    ++curr;

                    // value
                    char valBuf[32];
                    size_t len = 0;
                    while (curr < lineEnd && *curr != '\r' && len < 31) {
                        valBuf[len++] = (*curr == ',') ? '.' : *curr;
                        ++curr;
                    }
                    std::from_chars(valBuf, valBuf + len, m.value);

                    localVec.push_back(m);
                }

                p = lineEnd + 1; // Posun na další řádek
            }
        });
    }

    // Čekáme na dokončení všech vláken
    for (auto &t: threads) t.join();

    // Sériové spojení výsledků
    size_t totalMeasurements = 0;
    for (const auto &res: localResults) {
        totalMeasurements += res.size();
    }

    std::vector<Measurement> result;
    result.reserve(totalMeasurements);

    // Rychlý přesun naparsovaných dat z jednotlivých vláken do finálního vektoru
    for (auto &res: localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}

std::vector<Station> loadStationsParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    // Přeskočení hlavičky
    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};
    size_t startPos = headerEnd + 1;

    // Rozdělení na chunky pro vlákna
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *> > chunks;
    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        // Zarovnání na konec řádku
        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n') endPos++;
            if (endPos < size) endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    // Paralelní parsování
    std::vector<std::vector<Station> > localResults(chunks.size());
    std::vector<std::thread> threads;

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            // Odhad počtu stanic v chunku (předpoklad cca 50 znaků na stanici)
            localVec.reserve((end - p) / 50);

            while (p < end) {
                const char *lineEnd = p;
                while (lineEnd < end && *lineEnd != '\n') lineEnd++;

                if (lineEnd > p) {
                    Station s{};
                    const char *curr = p;

                    // ID
                    auto [ptr1, ec1] = std::from_chars(curr, lineEnd, s.id);
                    curr = ptr1;
                    if (curr < lineEnd && *curr == ';') curr++;

                    // Name (přeskočíme)
                    while (curr < lineEnd && *curr != ';') curr++;
                    if (curr < lineEnd && *curr == ';') curr++;

                    // Lat
                    auto [ptr2, ec2] = std::from_chars(curr, lineEnd, s.lat);
                    curr = ptr2;
                    if (curr < lineEnd && *curr == ';') curr++;

                    // Lon
                    std::from_chars(curr, lineEnd, s.lon);

                    localVec.push_back(s);
                }
                p = lineEnd + 1;
            }
        });
    }

    for (auto &t: threads) t.join();

    size_t totalStations = 0;
    for (const auto &res: localResults) totalStations += res.size();

    std::vector<Station> result;
    result.reserve(totalStations);

    for (auto &res: localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}
