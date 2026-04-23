#include "AppRunner.h"

#include <iostream>
#include <map>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <iterator>
#include <limits>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

#include "../processing/Filter.h"
#include "../processing/Aggregator.h"
#include "../processing/AnomalyDetector.h"
#include "../io/CsvWriter.h"
#include "../output/SvgWriter.h"
#include "../utils/Config.h"

void runSerial(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    std::cout << "Zpracovavam data (Seriove)...\n\n";

    // 1. Seskupení dat
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(stations.size());

    for (const auto &measurement: measurements) {
        groupedMeasurements[measurement.id][measurement.year].push_back(measurement);
    }

    // 2. Filtrování
    std::cout << "Filtrace data...\n";
    std::vector<int> passedFirstFilter = filterMinYears(groupedMeasurements, 5);
    std::vector<int> passedSecondFilter = filterMinReadings(groupedMeasurements, 100);
    std::cout << "Filtrace data dokončena!\n\n";

    std::ranges::sort(passedFirstFilter);
    std::ranges::sort(passedSecondFilter);

    std::vector<int> passedFilters;
    std::ranges::set_intersection(passedFirstFilter, passedSecondFilter, std::back_inserter(passedFilters));

    // 3. Výpočet průměrů
    std::cout << "Výpočet průměrů...\n";
    auto monthlyAverages = computeMonthlyAverages(groupedMeasurements, passedFilters);
    std::cout << "Výpočet průměrů dokončen!\n\n";

    // 4. Nalezení extrémů
    std::cout << "Hledání extrémů...\n";
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
    std::cout << "Hledání extrémů dokončeno!\n\n";

    // 5. Detekce a zápis anomálií
    std::cout << "Zpracovávání anomálií...\n";
    const std::vector<Anomaly> anomalies = detectAnomalies(monthlyAverages);
    writeSerialAnomaliesCsv(anomalies, Config::OUTPUT_SERIAL_FLUCTUATION_DIR);
    std::cout << "Zpracovávání anomálií dokončena!\n\n";

    // 6. Filtrace stanic pro mapy a zápis SVG
    std::cout << "Vytváření map...\n";
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());
    std::vector<Station> filteredStations;

    for (const auto &s: stations) {
        if (passedSet.contains(s.id)) {
            filteredStations.push_back(s);
        }
    }

    writeSvgMaps(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                 Config::OUTPUT_SERIAL_MAPS_DIR);
    std::cout << "Vytváření map dokončeno!\n\n";

    std::cout << "Seriove zpracovani hotovo!\n\n";
}

/** ================================================================================================================ */
/** ================================================================================================================ */
/** ================================================================================================================ */

void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    //std::cout << "Zpracovavam data (Paralelně)...\n\n";

    // 1. Seskupení dat (Paralelne)
    //std::cout << "Seskupovani dat (Paralelne)...\n";
    const size_t nThreads = std::thread::hardware_concurrency();

    // Použijeme tvůj koncept, ale vnější mapa je unordered_map (O(1) insert)
    std::vector<std::unordered_map<int, std::map<int, std::vector<Measurement> > > > localMaps(nThreads);

    std::vector<std::thread> threads;
    const size_t chunkSize = (measurements.size() + nThreads - 1) / nThreads;

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
    for (auto &thread: threads) thread.join();

    // Místo stavění setu z localMaps prostě využijeme vstupní vektor `stations`
    std::unordered_set<int> allStationIdsSet;
    for (const auto &localMap: localMaps) {
        for (const auto &sid: localMap | std::views::keys) {
            allStationIdsSet.insert(sid);
        }
    }

    std::vector<int> allStationIds(allStationIdsSet.begin(), allStationIdsSet.end());
    std::vector<std::map<int, std::vector<Measurement> > > mergedVec(allStationIds.size());
    std::vector<size_t> mergeIndices(allStationIds.size());
    std::iota(mergeIndices.begin(), mergeIndices.end(), 0);

    // Každý thread sloučí data jedné stanice ze všech local maps do svého slotu v mergedVec
    std::for_each(std::execution::par, mergeIndices.begin(), mergeIndices.end(), [&](size_t i) {
        const int sid = allStationIds[i];

        for (const auto &localMap: localMaps) {
            const auto it = localMap.find(sid);

            if (it == localMap.end()) continue;

            for (const auto &[year, ms]: it->second) {
                auto &target = mergedVec[i][year];
                target.insert(target.end(), ms.begin(), ms.end());
            }
        }
    });

    // Finální sloučení do výsledné struktury
    std::unordered_map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    groupedMeasurements.reserve(allStationIds.size());

    for (size_t i = 0; i < allStationIds.size(); i++) {
        if (!mergedVec[i].empty()) {
            groupedMeasurements[allStationIds[i]] = std::move(mergedVec[i]);
        }
    }
    //std::cout << "Seskupovani dokonceno!\n\n";

    // 2. Filtrování (jeden spojený průchod)
    //std::cout << "Filtrace data...\n";
    std::vector<int> passedFilters = filterStationsParallel(groupedMeasurements, 5, 100);
    //std::cout << "Filtrace data dokončena!\n\n";

    // 3. Výpočet průměrů
    //std::cout << "Výpočet průměrů...\n";
    auto monthlyAverages = computeMonthlyAveragesParallel(groupedMeasurements, passedFilters);
    //std::cout << "Výpočet průměrů dokončen!)\n\n";

    // 4. Nalezení extrémů
    //std::cout << "Hledání extrémů...\n";

    std::vector<int> activeStationIds;
    activeStationIds.reserve(monthlyAverages.size());
    for (const auto &sid: monthlyAverages | std::views::keys) activeStationIds.push_back(sid);

    // Každé vlákno dostane vlastní místo pro uložení {min, max}
    std::vector<std::pair<double, double> > localExtremes(activeStationIds.size(),
                                                          {
                                                              std::numeric_limits<double>::max(),
                                                              std::numeric_limits<double>::lowest()
                                                          });

    std::vector<size_t> indices(activeStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Každé vlákno zpracuje jednu stanici a uloží lokální min/max do svého slotu v localExtremes
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

    // Rychlý sériový merge výsledků vláken
    for (const auto &[fst, snd]: localExtremes) {
        if (fst < globalMin) globalMin = fst;
        if (snd > globalMax) globalMax = snd;
    }
    //std::cout << "Hledání extrémů dokončeno!\n\n";

    // 5. Detekce a zápis anomálií
    //std::cout << "Zpracovávání anomálií...\n";
    const std::vector<Anomaly> anomalies = detectAnomaliesParallel(monthlyAverages);
    writeParallelAnomaliesCsv(anomalies, Config::OUTPUT_PARALLEL_FLUCTUATION_DIR);
    //std::cout << "Zpracovávání anomálií dokončena!\n\n";

    // 6. Filtrace stanic pro mapy a zápis SVG
    //std::cout << "Vytváření map...\n";
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());

    std::vector<Station> filteredStations;
    filteredStations.resize(stations.size());

    auto it = std::copy_if(std::execution::par, stations.begin(), stations.end(), filteredStations.begin(),
                           [&](const Station &s) { return passedSet.contains(s.id); });

    filteredStations.erase(it, filteredStations.end());

    if (!std::filesystem::exists(Config::OUTPUT_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_DIR);
    }

    if (!std::filesystem::exists(Config::OUTPUT_PARALLEL_MAPS_DIR)) {
        std::filesystem::create_directories(Config::OUTPUT_PARALLEL_MAPS_DIR);
    }

    writeSvgMapsParallel(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH,
                         Config::OUTPUT_PARALLEL_MAPS_DIR);
    //std::cout << "Vytváření map dokončeno!\n\n";

    //std::cout << "Hotovo!\n\n";
}
