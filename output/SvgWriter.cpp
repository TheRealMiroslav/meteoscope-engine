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
 * @brief Vypočítá dlouhodobý průměr teploty pro konkrétní stanici a měsíc (napříč všemi roky).
 *
 * @param averages Vnořená mapa měření.
 * @param stationId ID cílové stanice.
 * @param month Cílový měsíc (1-12).
 *
 * @return double Vypočítaný teplotní průměr. Pokud nejsou data k dispozici, vrací 0.0.
 */
double getStationMonthAverage(
    const std::map<int, std::map<int, std::map<int, double> > > &averages,
    const int stationId, const int month) {
    // Ochrana před přístupem k neexistujícím datům stanice
    if (!averages.contains(stationId)) {
        return 0.0;
    }

    double sum = 0;
    int count = 0;

    // Iterace přes všechny roky dané stanice
    for (const auto &monthMap: averages.at(stationId) | std::views::values) {
        if (monthMap.contains(month)) {
            sum += monthMap.at(month);
            count++;
        }
    }

    if (count == 0) return 0.0;
    return sum / count;
}

/**
 * @brief Sekvenčně vygeneruje SVG mapy pro všech 12 měsíců v roce.
 *
 * Funkce načte podkladovou šablonu, a následně pro každý měsíc iteruje přes
 * všechny stanice, vypočítá jejich barvu a souřadnice a výsledné SVG uloží.
 *
 * @param filteredStations Vektor všech dostupných meteorologických stanic.
 * @param monthlyAverages Vnořená mapa obsahující průměrné teploty [stanice -> [rok -> [měsíc -> teplota]]].
 * @param globalMin Celkové teplotní minimum ze všech dat pro správné škálování barev.
 * @param globalMax Celkové teplotní maximum ze všech dat pro správné škálování barev.
 * @param mapSvgPath Cesta k podkladové SVG mapě, která slouží jako šablona.
 * @param outputDir Cílový adresář pro uložení vygenerovaných map.
 */
void writeSvgMapsSerial(const std::vector<Station> &filteredStations,
                        const std::map<int, std::map<int, std::map<int, double> > > &monthlyAverages, double globalMin,
                        double globalMax, const std::string &mapSvgPath, const std::string &outputDir) {
    // Automatické vytvoření výstupní složky, pokud neexistuje
    if (!outputDir.empty()) {
        fs::create_directories(outputDir);
    }

    std::string svgContent;
    std::ifstream fileStream(mapSvgPath);

    // Načtení podkladového SVG do paměti jako jeden textový řetězec
    if (!fileStream.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se nacist podkladovou mapu: " + mapSvgPath);
    }

    if (fileStream.is_open()) {
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        svgContent = buffer.str();
        fileStream.close();
    }

    for (int month = 1; month <= 12; month++) {
        // Názvy výstupních souborů
        constexpr const char *monthNames[] = {
            "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
            "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
        };

        std::string monthName = monthNames[month - 1];
        std::string svgMap = svgContent;

        // Použití ostringstream pro efektivní řetězení XML tagů generovaných v paměti
        std::ostringstream allCircles;

        for (const auto &station: filteredStations) {
            double avgTemp = getStationMonthAverage(monthlyAverages, station.id, month);

            // Mapování teploty na RGB hodnotu a geolokačních dat na SVG souřadnice
            auto [red, green, blue] = GetColor(avgTemp, globalMin, globalMax);
            auto [x, y] = GetCoordinates(station.lat, station.lon);

            // Vygenerování SVG elementu <circle> pro konkrétní stanici
            allCircles << "<circle cx=\"" << x
                    << "\" cy=\"" << y
                    << "\" r=\"" << Config::STATION_RADIUS
                    << "\" fill=\"rgb(" << red << "," << green << "," << blue << ")\"/>\n";
        }

        // Vyhledání uzavíracího tagu </svg> pro bezpečné vložení elementů
        size_t pos = svgMap.rfind("</svg>");
        if (pos != std::string::npos) {
            svgMap.insert(pos, allCircles.str());
        }

        // Zápis finálního obsahu do souboru pro aktuální měsíc
        std::string filePath = outputDir + "/" + monthName + ".svg";
        std::ofstream outputStream(filePath);
        outputStream << svgMap;
        outputStream.close();
    }
}

/**
 * @brief Paralelně vygeneruje SVG mapy pro všech 12 měsíců v roce.
 *
 * Pro dosažení vyššího výkonu je práce rozdělena pomocí std::execution::par.
 * Každé vlákno řeší jeden měsíc, provádí vlastní výpočty a samostatně zapisuje
 * do vlastního cílového souboru, což zajišťuje bezpečný běh bez nutnosti zámků (lock-free).
 *
 * @param filteredStations Vektor (filtrovaných) meteorologických stanic k vykreslení.
 * @param monthlyAverages Vnořená mapa obsahující průměrné teploty [stanice -> [rok -> [měsíc -> teplota]]].
 * @param globalMin Celkové teplotní minimum ze všech dat pro správné škálování barev.
 * @param globalMax Celkové teplotní maximum ze všech dat pro správné škálování barev.
 * @param mapSvgPath Cesta k podkladové SVG mapě, která slouží jako šablona.
 * @param outputDir Cílový adresář pro uložení vygenerovaných map.
 */
void writeSvgMapsParallel(
    const std::vector<Station> &filteredStations,
    const std::map<int, std::map<int, std::map<int, double> > > &monthlyAverages,
    double globalMin, double globalMax,
    const std::string &mapSvgPath,
    const std::string &outputDir) {
    // Automatické vytvoření výstupní složky
    if (!outputDir.empty()) {
        fs::create_directories(outputDir);
    }

    constexpr const char *monthNames[] = {
        "1_leden", "2_unor", "3_brezen", "4_duben", "5_kveten", "6_cerven",
        "7_cervenec", "8_srpen", "9_zari", "10_rijen", "11_listopad", "12_prosinec"
    };

    // Načtení šablony jen jednou pro všechny měsíce
    std::ifstream t(mapSvgPath);
    if (!t.is_open()) return;

    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string templateStr = buffer.str();

    // Rozdělení šablony na hlavičku a patičku před samotným generováním,
    // aby se nemusel string splitovat v každém vlákně zvlášť.
    size_t insertPos = templateStr.rfind("</svg>");
    if (insertPos == std::string::npos) insertPos = templateStr.length();

    std::string svgHeader = templateStr.substr(0, insertPos);
    std::string svgFooter = templateStr.substr(insertPos);

    std::vector<int> months = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    // Paralelní zpracování každého měsíce. Vlákna sdílí pouze read-only proměnné.
    std::for_each(std::execution::par, months.begin(), months.end(), [&](int month) {
        std::ostringstream allCircles;

        for (const auto &[id, lat, lon]: filteredStations) {
            auto it = monthlyAverages.find(id);
            if (it == monthlyAverages.end()) continue;

            double sum = 0.0;
            int count = 0;

            // Rychlý výpočet průměru in-line, šetří overhead volání externí funkce
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

                // Lokální stringstream bez rizika race condition mezi vlákny
                allCircles << "<circle cx=\"" << cx
                        << "\" cy=\"" << cy
                        << "\" r=\"" << Config::STATION_RADIUS
                        << "\" fill=\"rgb(" << r << "," << g << "," << b << ")\"/>\n";
            }
        }

        // Zápis výsledků specifických pro jedno vlákno a jeden měsíc
        const std::string outPath = outputDir + "/" + monthNames[month - 1] + ".svg";
        std::ofstream outFile(outPath);

        if (outFile.is_open()) {
            outFile << svgHeader << allCircles.str() << svgFooter;
        }
    });
}
