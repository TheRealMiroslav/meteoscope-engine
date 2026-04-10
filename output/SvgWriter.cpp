#include "SvgWriter.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <execution>
#include <ranges>
#include <unordered_map>

#include "ColorMapper.h"
#include "CoordMapper.h"
#include "../utils/Config.h"

void writeSvgMaps(const std::vector<Station> &stations,
                  const std::map<int, std::map<int, std::map<int, double> > > &averages, double globalMin,
                  double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    // 1. názvy měsíců
    const std::string monthNames[] = {
        "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
        "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
    };

    // 2. načti czmap.svg
    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

    if (fileStream.is_open()) {
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        svgContent = buffer.str();

        fileStream.close();
    }

    // 3. for month 1..12
    for (int month = 1; month <= 12; month++) {
        // 1. vytvoř název souboru pro tento měsíc
        std::string monthName = monthNames[month - 1];

        // 3a. zkopíruj svgContent
        std::string svgMap = svgContent;

        std::string circle;

        // 3b. for každá stanice
        for (const auto &station: stations) {
            // - getStationMonthAverage
            double avgTemp = getStationMonthAverage(averages, station.id, month);

            // - GetColor
            Color color = GetColor(avgTemp, globalMin, globalMax);

            // - GetCoordinates
            auto [x, y] = GetCoordinates(station.lat, station.lon);

            //- sestav <circle> string
            // <circle cx="600" cy="300" r="8" fill="rgb(255, 128, 0)"/>
            std::ostringstream circleStream;
            circleStream << "<circle cx=\"" << x
                    << "\" cy=\"" << y
                    << "\" r=\"" << Config::STATION_RADIUS
                    << "\" fill=\"rgb(" << color.r << "," << color.g << "," << color.b << ")\"/>\n";
            circle += circleStream.str();
        }

        //3c. vlož circles před </svg>
        size_t pos = svgMap.rfind("</svg>");
        svgMap.insert(pos, circle);

        //3d. zapiš do souboru
        std::string filePath = outputDir + "/" + monthName + ".svg";
        std::ofstream outputStream(filePath);
        outputStream << svgMap;
        outputStream.close();
    }
}

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

void writeSvgMapsParallel(const std::vector<Station> &stations,
                          const std::map<int, std::map<int, std::map<int, double> > > &averages, double globalMin,
                          double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    // 1. názvy měsíců
    const std::string monthNames[] = {
        "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
        "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
    };

    // 2. načti czmap.svg
    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

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

    std::vector months = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    std::vector<int> stationIdVec;
    stationIdVec.reserve(averages.size());
    for (const auto &stationId: averages | std::views::keys)
        stationIdVec.push_back(stationId);

    std::unordered_map<int, std::unordered_map<int, double>> stationMonthAvg;
    for (int stationId : stationIdVec)
        stationMonthAvg[stationId];

    std::for_each(std::execution::par, stationIdVec.begin(), stationIdVec.end(), [&](int stationId) {
        std::unordered_map<int, std::pair<double, int>> acc;
        for (const auto &monthMap: averages.at(stationId) | std::views::values)
            for (const auto &[month, val] : monthMap) {
                acc[month].first += val;
                acc[month].second++;
            }
        for (const auto &[month, p] : acc)
            stationMonthAvg[stationId][month] = p.first / p.second;
    });

    std::for_each(std::execution::par, months.begin(), months.end(), [&](int month) {
        std::string svgMap = svgContent;

        std::ostringstream allCircles;

        for (const auto &station: stations) {
            const double avgTemp = stationMonthAvg.at(station.id).at(month);
            auto [red, green, blue] = GetColor(avgTemp, globalMin, globalMax);
            auto [x, y] = coordCache.at(station.id);

            allCircles << "<circle cx=\"" << x
                    << "\" cy=\"" << y
                    << "\" r=\"" << Config::STATION_RADIUS
                    << "\" fill=\"rgb(" << red << "," << green << "," << blue << ")\"/>\n";
        }

        const size_t pos = svgMap.rfind("</svg>");
        svgMap.insert(pos, allCircles.str());

        const std::string filePath = outputDir + "/" + monthNames[month - 1] + ".svg";
        std::ofstream outputStream(filePath);
        outputStream << svgMap;
        outputStream.close();
    });
}