#include "AppRunner.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <execution>
#include <iterator>

#include <limits>
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
    std::map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    for (const auto &measurement: measurements) {
        groupedMeasurements[measurement.id][measurement.year].push_back(measurement);
    }

    // 2. Filtrování
    std::cout << "Filtrace data (Seriove)...\n";
    std::vector<int> passedFirstFilter = filterMinYears(groupedMeasurements, 5);
    std::vector<int> passedSecondFilter = filterMinReadings(groupedMeasurements, 100);
    std::cout << "Filtrace data dokončena! (Seriove)\n\n";

    std::ranges::sort(passedFirstFilter);
    std::ranges::sort(passedSecondFilter);

    std::vector<int> passedFilters;
    std::ranges::set_intersection(passedFirstFilter, passedSecondFilter, std::back_inserter(passedFilters));

    // 3. Výpočet průměrů
    std::cout << "Výpočet průměrů (Seriove)...\n";
    auto monthlyAverages = computeMonthlyAverages(groupedMeasurements, passedFilters);
    std::cout << "Výpočet průměrů dokončen! (Seriove)\n\n";

    // 4. Nalezení extrémů
    std::cout << "Hledání extrémů (Seriove)...\n";
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = std::numeric_limits<double>::lowest();

    for (auto const &[stationId, yearMap]: monthlyAverages) {
        for (auto const &[year, monthMap]: yearMap) {
            for (auto const &[month, avg]: monthMap) {
                if (avg < globalMin) globalMin = avg;
                if (avg > globalMax) globalMax = avg;
            }
        }
    }
    std::cout << "Hledání extrémů dokončeno! (Seriove)\n\n";

    // 5. Detekce a zápis anomálií
    std::cout << "Zpracovávání anomálií (Seriove)...\n";
    const std::vector<Anomaly> anomalies = detectAnomalies(monthlyAverages);
    writeAnomaliesCsv(anomalies, "./vykyvy.csv");
    std::cout << "Zpracovávání anomálií dokončena! (Seriove)\n\n";

    // 6. Filtrace stanic pro mapy a zápis SVG
    std::cout << "Vytváření map (Seriove)...\n";
    std::vector<Station> filteredStations;
    for (const auto &s: stations) {
        if (std::ranges::find(passedFilters, s.id) != passedFilters.end()) {
            filteredStations.push_back(s);
        }
    }
    writeSvgMaps(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH, "./maps/");
    std::cout << "Vytváření map dokončeno! (Seriove)\n\n";

    std::cout << "Hotovo!\n\n";
}

void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements) {
    std::cout << "Zpracovavam data (Paralelne)...\n\n";

    // 1. Seskupení dat
    const size_t nThreads = std::thread::hardware_concurrency();
    std::vector<std::map<int, std::map<int, std::vector<Measurement> > > > localMaps(nThreads);

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

    // Správný merge — sloučit vektory, ne přepsat
    std::map<int, std::map<int, std::vector<Measurement> > > groupedMeasurements;
    for (const auto &localMap: localMaps) {
        for (const auto &[stationId, yearMap]: localMap) {
            for (const auto &[year, ms]: yearMap) {
                auto &target = groupedMeasurements[stationId][year];
                target.insert(target.end(), ms.begin(), ms.end());
            }
        }
    }

    // 2. Filtrování
    std::cout << "Filtrace data (Paralelne)...\n";
    std::vector<int> passedFirstFilter = filterMinYearsParallel(groupedMeasurements, 5);
    std::vector<int> passedSecondFilter = filterMinReadingsParallel(groupedMeasurements, 100);
    std::cout << "Filtrace data dokončena! (Paralelne)\n\n";

    std::ranges::sort(passedFirstFilter);
    std::ranges::sort(passedSecondFilter);

    std::vector<int> passedFilters;
    std::ranges::set_intersection(passedFirstFilter, passedSecondFilter, std::back_inserter(passedFilters));

    // 3. Výpočet průměrů
    std::cout << "Výpočet průměrů (Paralelne)...\n";
    auto monthlyAverages = computeMonthlyAveragesParallel(groupedMeasurements, passedFilters);
    std::cout << "Výpočet průměrů dokončen! (Paralelne)\n\n";

    // 4. Nalezení extrémů
    std::cout << "Hledání extrémů (Paralelne)...\n";
    // Flatten do vektoru a pak parallel reduce
    std::vector<double> allValues;
    for (auto const &[sid, yearMap]: monthlyAverages)
        for (auto const &[year, monthMap]: yearMap)
            for (auto const &[month, avg]: monthMap)
                allValues.push_back(avg);

    double globalMin = 0.0;
    double globalMax = 0.0;
    if (!allValues.empty()) {
        auto [itMin, itMax] = std::minmax_element(
            std::execution::par, allValues.begin(), allValues.end());
        globalMin = *itMin;
        globalMax = *itMax;
    }
    std::cout << "Hledání extrémů dokončeno! (Paralelne)\n\n";

    // 5. Detekce a zápis anomálií
    std::cout << "Zpracovávání anomálií (Paralelne)...\n";
    const std::vector<Anomaly> anomalies = detectAnomaliesParallel(monthlyAverages);
    writeAnomaliesCsv(anomalies, "./vykyvy.csv");
    std::cout << "Zpracovávání anomálií dokončena! (Paralelne)\n\n";

    // 6. Filtrace stanic pro mapy a zápis SVG
    std::cout << "Vytváření map (Paralelne)...\n";
    const std::unordered_set<int> passedSet(passedFilters.begin(), passedFilters.end());

    std::vector<Station> filteredStations;
    for (const auto &s: stations) {
        if (passedSet.contains(s.id)) {
            filteredStations.push_back(s);
        }
    }
    writeSvgMapsParallel(filteredStations, monthlyAverages, globalMin, globalMax, Config::MAP_SVG_PATH, "../maps/");
    std::cout << "Vytváření map dokončeno! (Paralelne)\n\n";

    std::cout << "Hotovo!\n\n";
}
