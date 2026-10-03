#include "SvgWriter.h"

#include <vector>
#include <string>
#include <ranges>
#include <map>
#include <execution>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <utility>

#include "ColorMapper.h"
#include "CoordMapper.h"
#include "../utils/Config.h"

namespace fs = std::filesystem;

/**
 * @brief Computes long-term average temperature for a specific station and month across all recorded years.
 *
 * @param averages Nested map of observations.
 * @param stationId Target station identifier.
 * @param month Target calendar month (1-12).
 *
 * @return double Calculated mean temperature. Returns 0.0 if no observations are available.
 */
double getStationMonthAverage(const std::map<int, std::map<int, std::map<int, double>>> &averages, const int stationId,
                              const int month) {
    if (!averages.contains(stationId)) {
        return 0.0;
    }

    double sum = 0.0;
    int count = 0;

    for (const auto &monthMap : averages.at(stationId) | std::views::values) {
        if (monthMap.contains(month)) {
            sum += monthMap.at(month);
            count++;
        }
    }

    if (count == 0)
        return 0.0;
    return sum / count;
}

/**
 * @brief Sequentially generates SVG maps for all 12 calendar months.
 *
 * Reads base SVG template, iterates through stations for each month,
 * evaluates projected coordinates and RGB colors, and writes output files.
 *
 * @param filteredStations Vector of validated meteorological stations.
 * @param monthlyAverages Nested map of mean temperatures [station -> [year -> [month -> temperature]]].
 * @param globalMin Global temperature minimum across entire dataset for color scaling.
 * @param globalMax Global temperature maximum across entire dataset for color scaling.
 * @param mapSvgPath Path to base SVG template file.
 * @param outputDir Target directory for generated maps.
 */
void writeSvgMapsSerial(const std::vector<Station> &filteredStations,
                        const std::map<int, std::map<int, std::map<int, double>>> &monthlyAverages, double globalMin,
                        double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    if (!outputDir.empty()) {
        fs::create_directories(outputDir);
    }

    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

    if (!fileStream.is_open()) {
        throw std::runtime_error("Error: Failed to load base map SVG: " + mapSvgPath);
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    svgContent = buffer.str();
    fileStream.close();

    constexpr const char *monthNames[] = {"01_january",   "02_february", "03_march",    "04_april",
                                          "05_may",       "06_june",     "07_july",     "08_august",
                                          "09_september", "10_october",  "11_november", "12_december"};

    for (int month = 1; month <= 12; month++) {
        std::string monthName = monthNames[month - 1];
        std::string svgMap = svgContent;

        std::ostringstream allCircles;

        for (const auto &station : filteredStations) {
            double avgTemp = getStationMonthAverage(monthlyAverages, station.id, month);

            auto [red, green, blue] = GetColor(avgTemp, globalMin, globalMax);
            auto [x, y] = GetCoordinates(station.lat, station.lon);

            allCircles << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << Config::STATION_RADIUS
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

/**
 * @brief Concurrently generates SVG maps for all 12 calendar months.
 *
 * Distributes month generation across threads using std::execution::par.
 * Each worker operates on dedicated string streams and writes to isolated files (lock-free).
 *
 * @param filteredStations Vector of validated meteorological stations.
 * @param monthlyAverages Nested map of mean temperatures [station -> [year -> [month -> temperature]]].
 * @param globalMin Global temperature minimum across entire dataset for color scaling.
 * @param globalMax Global temperature maximum across entire dataset for color scaling.
 * @param mapSvgPath Path to base SVG template file.
 * @param outputDir Target directory for generated maps.
 */
void writeSvgMapsParallel(const std::vector<Station> &filteredStations,
                          const std::map<int, std::map<int, std::map<int, double>>> &monthlyAverages, double globalMin,
                          double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    if (!outputDir.empty()) {
        fs::create_directories(outputDir);
    }

    constexpr const char *monthNames[] = {"01_january",   "02_february", "03_march",    "04_april",
                                          "05_may",       "06_june",     "07_july",     "08_august",
                                          "09_september", "10_october",  "11_november", "12_december"};

    std::ifstream fileStream(mapSvgPath);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Error: Failed to load base map SVG: " + mapSvgPath);
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string templateStr = buffer.str();

    size_t insertPos = templateStr.rfind("</svg>");
    if (insertPos == std::string::npos)
        insertPos = templateStr.length();

    std::string svgHeader = templateStr.substr(0, insertPos);
    std::string svgFooter = templateStr.substr(insertPos);

    std::vector<int> months = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    std::for_each(std::execution::par, months.begin(), months.end(), [&](int month) {
        std::ostringstream allCircles;

        for (const auto &[id, lat, lon] : filteredStations) {
            auto it = monthlyAverages.find(id);
            if (it == monthlyAverages.end())
                continue;

            double sum = 0.0;
            int count = 0;

            for (const auto &monthMap : it->second | std::views::values) {
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

                allCircles << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << Config::STATION_RADIUS
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
