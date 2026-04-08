#include "SvgWriter.h"

#include <fstream>
#include <sstream>

#include "ColorMapper.h"
#include "CoordMapper.h"
#include "../utils/Config.h"

void writeSvgMaps(const std::vector<Station> &stations,
                  const std::map<int, std::map<int, std::map<int, double>> > &averages, double globalMin,
                  double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    // 1. názvy měsíců
    const std::string monthNames[] = {
        "leden", "únor", "březen", "duben", "květen", "červen",
        "červenec", "srpen", "září", "říjen", "listopad", "prosinec"
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

        std::string circle = "";

        // 3b. for každá stanice
        for (const auto &station : stations) {
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
    const std::map<int, std::map<int, std::map<int, double>>> &averages,
    const int stationId, const int month) {

    double sum = 0;
    int count = 0;

    for (auto const& [year, monthMap] : averages.at(stationId)) {

        if (monthMap.count(month) > 0) {
            sum += monthMap.at(month);
            count++;
        }
    }

    if (count == 0) return 0.0;
    return sum / count;
}
