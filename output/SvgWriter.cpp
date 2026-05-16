#include "SvgWriter.h"

#include <vector>
#include <string>
#include <ranges>
#include <unordered_map>
#include <map>
#include <array>
#include <execution>
#include <numeric>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <cstdint>

#include "ColorMapper.h"
#include "CoordMapper.h"
#include "../utils/Config.h"

double getStationMonthAverage(
    const std::map<int, std::map<int, std::map<int, double> > > &averages,
    const int stationId, const int month) {
    if (!averages.contains(stationId)) {
        return 0.0;
    }

    double sum = 0;
    int count = 0;

    for (const auto &monthMap: averages.at(stationId) | std::views::values) {
        if (monthMap.contains(month)) {
            sum += monthMap.at(month);
            count++;
        }
    }

    if (count == 0) return 0.0;
    return sum / count;
}

void writeSvgMaps(const std::vector<Station> &stations,
                  const std::map<int, std::map<int, std::map<int, double> > > &averages, double globalMin,
                  double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    // 2. načti czmap.svg
    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

    if (!fileStream.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se nacist podkladovou mapu: " + mapSvgPath);
    }

    if (fileStream.is_open()) {
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        svgContent = buffer.str();
        fileStream.close();
    }

    // 3. for month 1..12
    for (int month = 1; month <= 12; month++) {
        // 1. názvy měsíců
        constexpr const char *monthNames[] = {
            "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
            "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
        };

        std::string monthName = monthNames[month - 1];
        std::string svgMap = svgContent;

        std::ostringstream allCircles;

        for (const auto &station: stations) {
            double avgTemp = getStationMonthAverage(averages, station.id, month);
            auto [red, green, blue] = GetColor(avgTemp, globalMin, globalMax);
            auto [x, y] = GetCoordinates(station.lat, station.lon);

            allCircles << "<circle cx=\"" << x
                    << "\" cy=\"" << y
                    << "\" r=\"" << Config::STATION_RADIUS
                    << "\" fill=\"rgb(" << red << "," << green << "," << blue << ")\"/>\n";
        }

        size_t pos = svgMap.rfind("</svg>");
        if (pos != std::string::npos) {
            svgMap.insert(pos, allCircles.str());
        }

        std::string filePath = outputDir + "/" + monthName + ".svg";
        std::ofstream outputStream(filePath);
        outputStream << svgMap;
        outputStream.close();
    }
}

void writeSvgMapsParallel(const std::vector<Station> &stations,
                          const std::map<int, std::map<int, std::map<int, double> > > &averages, double globalMin,
                          double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    constexpr const char *monthNames[] = {
        "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
        "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
    };

    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

    if (!fileStream.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se nacist podkladovou mapu: " + mapSvgPath);
    }

    if (fileStream.is_open()) {
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        svgContent = buffer.str();
        fileStream.close();
    }

    std::unordered_map<int, std::pair<int, int> > coordCache;
    for (const auto &station: stations) {
        coordCache[station.id] = GetCoordinates(station.lat, station.lon);
    }

    std::vector<int> stationIdVec;
    stationIdVec.reserve(averages.size());
    for (const auto &stationId: averages | std::views::keys) {
        stationIdVec.push_back(stationId);
    }

    // Bezpečná paralelní struktura (index 1..12 pro měsíce, pole inicializováno na 0)
    std::vector<std::array<double, 13> > threadAvgResults(stationIdVec.size());
    for (auto &arr: threadAvgResults) arr.fill(0.0);

    std::vector<size_t> indices(stationIdVec.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Paralelní výpočet průměrů per stanice bez konfliktů zápisu (Data Race volné)
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = stationIdVec[i];
        std::array<double, 13> sums{0};
        std::array<int, 13> counts{0};

        for (const auto &monthMap: averages.at(stationId) | std::views::values) {
            for (const auto &[month, val]: monthMap) {
                sums[month] += val;
                counts[month]++;
            }
        }

        for (int m = 1; m <= 12; ++m) {
            if (counts[m] > 0) {
                threadAvgResults[i][m] = sums[m] / counts[m];
            }
        }
    });

    // Rychlé sériové přelití do mapy pro O(1) vyhledávání
    std::unordered_map<int, std::array<double, 13> > stationMonthAvg;
    for (size_t i = 0; i < stationIdVec.size(); ++i) {
        stationMonthAvg[stationIdVec[i]] = threadAvgResults[i];
    }

    std::vector<int> months = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    std::for_each(std::execution::par, months.begin(), months.end(), [&](int month) {
        std::string svgMap = svgContent;
        std::ostringstream allCircles;

        for (const auto &station: stations) {
            // Kontrola existence zamezí případnému pádu
            auto it = stationMonthAvg.find(station.id);
            if (it != stationMonthAvg.end()) {
                const double avgTemp = it->second[month];
                auto [red, green, blue] = GetColor(avgTemp, globalMin, globalMax);
                auto [x, y] = coordCache.at(station.id);

                allCircles << "<circle cx=\"" << x
                        << "\" cy=\"" << y
                        << "\" r=\"" << Config::STATION_RADIUS
                        << "\" fill=\"rgb(" << red << "," << green << "," << blue << ")\"/>\n";
            }
        }

        size_t pos = svgMap.rfind("</svg>");
        if (pos != std::string::npos) {
            svgMap.insert(pos, allCircles.str());
        }

        const std::string filePath = outputDir + "/" + monthNames[month - 1] + ".svg";
        std::ofstream outputStream(filePath);
        outputStream << svgMap;
        outputStream.close();
    });
}

/**
 * @brief Vylepšená paralelní verze pro zápis SVG map, která minimalizuje režii a zamezuje datovým konfliktům.
 *
 * @param filteredStations - pouze stanice, které prošly filtry (pro zrychlení)
 * @param monthlyAverages
 * @param globalMin
 * @param globalMax
 * @param templatePath
 * @param outputDir
 */
void writeSvgMapsParallelOptimized(
    const std::vector<Station> &filteredStations,
    const std::map<int, std::map<int, std::map<int, double> > > &monthlyAverages,
    double globalMin, double globalMax,
    const std::string &templatePath,
    const std::string &outputDir) {

    constexpr const char *monthNames[] = {
        "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
        "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
    };

    std::ifstream t(templatePath);
    if (!t.is_open()) return;

    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string templateStr = buffer.str();

    size_t insertPos = templateStr.rfind("</svg>");
    if (insertPos == std::string::npos) insertPos = templateStr.length();

    std::string svgHeader = templateStr.substr(0, insertPos);
    std::string svgFooter = templateStr.substr(insertPos);

    std::vector<int> months = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    // Paralelizujeme napříč měsíci (max 12 vláken)
    std::for_each(std::execution::par, months.begin(), months.end(), [&](int month) {
        std::ostringstream allCircles;

        for (const auto &[id, lat, lon] : filteredStations) {
            auto it = monthlyAverages.find(id);
            if (it == monthlyAverages.end()) continue;

            double sum = 0.0;
            int count = 0;

            for (const auto &monthMap: it->second | std::views::values) {
                auto mIt = monthMap.find(month);

                if (mIt != monthMap.end()) {
                    sum += mIt->second;
                    count++;
                }
            }

            if (count > 0) {
                const double avg = sum / count;
                const auto [r, g, b] = GetColor(avg, globalMin, globalMax);
                auto [cx, cy] = GetCoordinates(lat, lon);

                allCircles << "<circle cx=\"" << cx
                           << "\" cy=\"" << cy
                           << "\" r=\"" << Config::STATION_RADIUS
                           << "\" fill=\"rgb(" << r << "," << g << "," << b << ")\"/>\n";
            }
        }

        const std::string outPath = outputDir + "/" + monthNames[month - 1] + ".svg";
        std::ofstream outFile(outPath);

        if (outFile.is_open()) {
            outFile << svgHeader << allCircles.str() << svgFooter;
        }
    });
}
