#include "AppRunner.h"

#include <map>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <limits>
#include <ranges>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "../processing/Filter.h"
#include "../processing/Aggregator.h"
#include "../processing/AnomalyDetector.h"
#include "../io/CsvWriter.h"
#include "../output/SvgWriter.h"
#include "../utils/Config.h"

/**
 * @brief Sériová verze hlavního procesu pro zpracování meteorologických dat.
 *
 * Funkce postupně seskupí data, vyfiltruje validní stanice (podle počtu let a záznamů),
 * vypočítá měsíční průměry, najde globální extrémy, detekuje anomálie a následně
 * vygeneruje CSV reporty a SVG mapy. Vše probíhá sekvenčně v jednom vlákně.
 *
 * @param stations Vektor všech dostupných meteorologických stanic.
 * @param measurements Vektor všech naměřených hodnot ke zpracování.
 */
void runSerial(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    // 1. Seskupení dat
    // Vytvoření struktury: ID stanice -> (Rok -> Naměřené hodnoty)
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(stations.size());

    for (const auto &measurement: measurements) {
        groupedMeasurements[measurement.id][measurement.year].push_back(measurement);
    }

    // 2. Filtrování (jeden spojený průchod)
    // Očištění dat od stanic, které nemají dostatečnou historii nebo hustotu měření
    std::vector<int> passedFilters = filterStationsSerial(groupedMeasurements, 5, 100);

    // 3. Výpočet průměrů
    // Agregace naměřených hodnot na měsíční bázi pro validní stanice
    auto monthlyAverages = computeMonthlyAverages(groupedMeasurements, passedFilters);

    // 4. Nalezení extrémů
    // Vyhledání absolutního minima a maxima ze všech vypočítaných průměrů pro správné nastavení barevné škály map
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    for (const auto &yearMap: monthlyAverages | std::views::values) {
        for (const auto &monthMap: yearMap | std::views::values) {
            for (const auto &avg: monthMap | std::views::values) {
                if (avg < globalMin) globalMin = avg;
                if (avg > globalMax) globalMax = avg;
            }
        }
    }

    // 5. Detekce a zápis anomálií
    // Vyhodnocení průměrů, nalezení výkyvů (anomálií) a export do CSV reportu
    const std::vector<Anomaly> anomalies = detectAnomalies(monthlyAverages);
    writeSerialAnomaliesCsv(anomalies, Config::OUTPUT_SERIAL_FLUCTUATION_DIR);

    // 6. Filtrace stanic pro mapy a zápis SVG
    // Odfiltrování stanic, které neprošly počátečními filtry, pro potřeby vizualizace
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());
    std::vector<Station> filteredStations;

    for (const auto &s: stations) {
        if (passedSet.contains(s.id)) {
            filteredStations.push_back(s);
        }
    }

    // Vykreslení SVG map s využitím zjištěných extrémů pro normalizaci teplotní škály
    writeSvgMapsSerial(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                       Config::OUTPUT_SERIAL_MAPS_DIR);
}

/**
 * @brief Paralelní verze hlavního procesu pro zpracování meteorologických dat.
 *
 * Funkce provádí stejné kroky jako `runSerial`, ale využívá vícevláknové zpracování
 * (std::thread, std::execution::par) k optimalizaci výpočtů a agregace dat.
 * Obsahuje map-reduce logiku pro bezpečné rozdělení a sloučení dat mezi vlákny.
 *
 * @param stations Vektor všech dostupných meteorologických stanic.
 * @param measurements Vektor všech naměřených hodnot ke zpracování.
 */
void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    // 1. Seskupení dat (Paralelne)
    // Zjištění počtu dostupných hardwarových vláken (minimálně 1)
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

    // Každé vlákno bude mít vlastní instanci vnější unordered_map pro zamezení datových závodů (data races).
    // Použití unordered_map poskytuje asymptotickou složitost O(1) pro vložení.
    std::vector<std::unordered_map<int, std::map<int, std::vector<Measurement> > > > localMaps(nThreads);

    std::vector<std::thread> threads;
    // Výpočet velikosti bloku dat připadajícího na jedno vlákno
    const size_t chunkSize = (measurements.size() + nThreads - 1) / nThreads;

    // Fáze mapování: Rozdělení vstupních měření na bloky (chunks) a jejich zpracování v oddělených vláknech
    for (size_t t = 0; t < nThreads; t++) {
        threads.emplace_back([&, t]() {
            const size_t start = t * chunkSize;
            const size_t end = std::min(start + chunkSize, measurements.size());

            for (size_t i = start; i < end; i++) {
                const auto &m = measurements[i];
                localMaps[t][m.id][m.year].push_back(m);
            }
        });
    }

    // Vyčkání na dokončení všech vláken
    for (auto &thread: threads) thread.join();

    // Extrakce všech unikátních ID stanic ze všech lokálních map (agregace nalezených klíčů)
    std::unordered_set<int> allStationIdsSet;
    for (const auto &localMap: localMaps) {
        for (const auto &sid: localMap | std::views::keys) {
            allStationIdsSet.insert(sid);
        }
    }

    std::vector<int> allStationIds(allStationIdsSet.begin(), allStationIdsSet.end());
    // Předalokace výsledného vektoru pro fázovou redukci (každý index odpovídá jedné stanici)
    std::vector<std::map<int, std::vector<Measurement> > > mergedVec(allStationIds.size());
    std::vector<size_t> mergeIndices(allStationIds.size());
    std::iota(mergeIndices.begin(), mergeIndices.end(), 0); // Naplnění indexy 0, 1, ..., N-1

    // Fáze redukce: Paralelní procházení přes indexy stanic.
    // Každé vlákno zpracuje jednu stanici a agreguje její data ze VŠECH lokálních vláknových map do jednoho výsledku.
    std::for_each(std::execution::par, mergeIndices.begin(), mergeIndices.end(), [&](size_t i) {
        const int sid = allStationIds[i];

        for (const auto &localMap: localMaps) {
            const auto it = localMap.find(sid);

            if (it == localMap.end()) continue;

            for (const auto &[year, ms]: it->second) {
                auto &target = mergedVec[i][year];

                // Sloučení (append) vektorů měření pro daný rok
                target.insert(target.end(), ms.begin(), ms.end());
            }
        }
    });

    // Finální sestavení hlavní struktury groupedMeasurements přesunem (move sémantika) agregovaných dat z redukce
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(allStationIds.size());

    for (size_t i = 0; i < allStationIds.size(); i++) {
        if (!mergedVec[i].empty()) {
            groupedMeasurements[allStationIds[i]] = std::move(mergedVec[i]);
        }
    }

    // 2. Filtrování (jeden spojený průchod)
    // Multivláknová filtrace stanic na základě minimálních požadavků (5 let, 100 záznamů celkově)
    std::vector<int> passedFilters = filterStationsParallel(groupedMeasurements, 5, 100);

    // 3. Výpočet průměrů
    // Paralelní zpracování výpočtu měsíčních průměrů teplot pro vyfiltrované stanice
    auto monthlyAverages = computeMonthlyAveragesParallel(groupedMeasurements, passedFilters);

    // 4. Nalezení extrémů
    std::vector<int> activeStationIds;
    activeStationIds.reserve(monthlyAverages.size());
    for (const auto &sid: monthlyAverages | std::views::keys) activeStationIds.push_back(sid);

    // Příprava struktury pro lokální extrémy. Každé vlákno si zapíše lokální {min, max} na index odpovídající jeho zpracovávané stanici.
    // Tímto se zamezuje potřebě zámků (mutexů) při paralelizaci.
    std::vector<std::pair<double, double> > localExtremes(activeStationIds.size(), {
                                                              std::numeric_limits<double>::max(),
                                                              std::numeric_limits<double>::lowest()
                                                          });

    std::vector<size_t> indices(activeStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Výpočet lokálních extrémů paralelně přes všechny stanice
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        double lMin = std::numeric_limits<double>::max();
        double lMax = std::numeric_limits<double>::lowest();

        for (const auto &monthMap: monthlyAverages.at(activeStationIds[i]) | std::views::values) {
            for (const auto &avg: monthMap | std::views::values) {
                if (avg < lMin) lMin = avg;
                if (avg > lMax) lMax = avg;
            }
        }

        localExtremes[i] = {lMin, lMax};
    });

    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    // Rychlý sekvenční průchod (redukce) přes shromážděná lokální minima a maxima k zisku konečného globálního extrému.
    for (const auto &[fst, snd]: localExtremes) {
        if (fst < globalMin) globalMin = fst;
        if (snd > globalMax) globalMax = snd;
    }

    // 5. Detekce a zápis anomálií
    // Identifikace teplotních anomálií pomocí paralelní implementace a asynchronní/blokový zápis do CSV
    const std::vector<Anomaly> anomalies = detectAnomaliesParallel(monthlyAverages);
    writeParallelAnomaliesCsv(anomalies, Config::OUTPUT_PARALLEL_FLUCTUATION_DIR);

    // 6. Filtrace stanic pro mapy a zápis SVG
    // Vytvoření hash setu pro vysoce výkonné O(1) vyhledávání propustných stanic
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());

    std::vector<Station> filteredStations;
    filteredStations.resize(stations.size());

    // Paralelní filtrování vstupního vektoru stanic dle passedSet s přesunem do vektoru filteredStations
    auto it = std::copy_if(std::execution::par, stations.begin(), stations.end(), filteredStations.begin(),
                           [&](const Station &s) { return passedSet.contains(s.id); });

    // Odstranění volného nevyužitého místa vzniklého po filtraci
    filteredStations.erase(it, filteredStations.end());

    // Zajištění existence výstupních složek pro zápis
    if (!std::filesystem::exists(Config::OUTPUT_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_DIR);
    }

    if (!std::filesystem::exists(Config::OUTPUT_PARALLEL_MAPS_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_PARALLEL_MAPS_DIR);
    }

    // Spuštění vysoce optimalizovaného paralelního zápisu SVG souborů
    writeSvgMapsParallel(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                         Config::OUTPUT_PARALLEL_MAPS_DIR);
}
