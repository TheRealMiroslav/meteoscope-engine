#include "Aggregator.h"

#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <execution>
#include <array>
#include <unordered_map>

/**
 * @brief Sekvenčně vypočítá měsíční průměry teplot pro zadané stanice.
 *
 * @param groupedMeasurements Naměřená data seskupená podle stanice, roku a měsíce.
 * @param passedStationIds Seznam ID stanic, pro které se mají průměry počítat.
 *
 * @return std::map<int, std::map<int, std::map<int, double>>> Vnořená mapa obsahující
 * výsledné průměry ve formátu [ID stanice -> [Rok -> [Měsíc -> Průměrná teplota]]].
 */
std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds) {
    // Použití unordered_set pro vyhledávání povolených stanic v čase O(1)
    const std::unordered_set<int> allowedStations(passedStationIds.begin(), passedStationIds.end());

    std::map<int, std::map<int, std::map<int, double> > > results;

    for (auto const &[stationId, yearMap]: groupedMeasurements) {
        // Ignorování stanic, které nebyly vyžádány ke zpracování
        if (!allowedStations.contains(stationId)) continue;

        for (auto const &[year, measurements]: yearMap) {
            // Indexy 1-12 odpovídají měsícům (index 0 se ignoruje)
            // first = suma hodnot, second = počet měření
            std::array<std::pair<double, int>, 13> statsPerMonth{};

            // Agregace sum a počtů v jediném průchodu lineárně
            for (const auto &measurement: measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            // Výpočet finálního průměru pro měsíce, které mají data
            for (int month = 1; month <= 12; ++month) {
                if (statsPerMonth[month].second > 0) {
                    const double average = statsPerMonth[month].first / statsPerMonth[month].second;
                    results[stationId][year][month] = average;
                }
            }
        }
    }

    return results;
}

/**
 * @brief Paralelně vypočítá měsíční průměry teplot pro zadané stanice.
 *
 * Efektivnější varianta pro velká množství dat. Výpočet je paralelizován na úrovni
 * jednotlivých stanic, přičemž každé vlákno zpracovává všechny roky a měsíce dané stanice.
 *
 * @param groupedMeasurements Naměřená data seskupená podle stanice, roku a měsíce.
 * @param passedStationIds Seznam ID stanic, pro které se mají průměry počítat.
 *
 * @return std::map<int, std::map<int, std::map<int, double>>> Vnořená mapa obsahující
 * výsledné průměry ve formátu [ID stanice -> [Rok -> [Měsíc -> Průměrná teplota]]].
 */
std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAveragesParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds) {
    // Příprava indexů pro paralelní spuštění (std::for_each potřebuje iterátory)
    std::vector<int> indices(passedStationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Definice pomocného typu pro uchování mezivýsledků každého vlákna (vyhnutí se lockování sdílené mapy)
    using StationResult = std::map<int, std::map<int, double> >;
    std::vector<StationResult> threadResult(passedStationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = passedStationIds.at(i);
        const auto &yearMap = groupedMeasurements.at(stationId);

        StationResult localStationResult;

        for (auto const &[year, measurements]: yearMap) {
            std::array<std::pair<double, int>, 13> statsPerMonth{};

            for (const auto &measurement: measurements) {
                statsPerMonth[measurement.month].first += measurement.value;
                statsPerMonth[measurement.month].second += 1;
            }

            for (int month = 1; month <= 12; ++month) {
                if (statsPerMonth[month].second > 0) {
                    const double average = statsPerMonth[month].first / statsPerMonth[month].second;
                    localStationResult[year][month] = average;
                }
            }
        }
        // Uložení vypočítaných dat stanice na pozici odpovídající indexu vlákna (bezpečný move semantics)
        threadResult[i] = std::move(localStationResult);
    });

    // Sekvenční sestavení finálního výsledku z dat předpřipravených vlákny
    std::map<int, std::map<int, std::map<int, double> > > finalResults;
    for (size_t i = 0; i < passedStationIds.size(); i++) {
        finalResults[passedStationIds[i]] = std::move(threadResult[i]);
    }

    return finalResults;
}
