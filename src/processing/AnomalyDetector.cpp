#include "AnomalyDetector.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>
#include <limits>
#include <ranges>
#include <tuple>
#include <iterator>

/**
 * @brief Sekvenčně detekuje teplotní anomálie pro všechny stanice.
 *
 * @param averages Vnořená mapa měsíčních průměrů [ID stanice -> [Rok -> [Měsíc -> Teplota]]].
 *
 * @return std::vector<Anomaly> Seznam detekovaných anomálií seřazený chronologicky a dle stanic.
 */
std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages) {
    std::vector<Anomaly> result;

    for (const auto &[stationId, yearMap]: averages) {
        // Lokální MIN a MAX pro danou stanici, indexy 1-12 odpovídají měsícům
        std::array<double, 13> minVals{};
        std::array<double, 13> maxVals{};
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        // Nalezení globálních minim a maxim v daném měsíci pro celou historii stanice
        for (const auto &monthMap: yearMap | std::views::values) {
            for (const auto &[month, avg]: monthMap) {
                if (avg < minVals[month]) minVals[month] = avg;
                if (avg > maxVals[month]) maxVals[month] = avg;
            }
        }

        // Předvypočítáme si dynamické thresholdy (hraniční hodnoty pro anomálie).
        // Zde považujeme za anomálii výkyv o velikosti 75 % historického teplotního rozpětí.
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // Proměnná pro ukládání stavu předchozího zpracovaného roku a teploty
        // pair = {Rok, Průměrná teplota}
        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &fst: prevMonthData | std::views::keys) fst = -1; // -1 = zatím neznámý rok

        // Chronologická detekce výkyvů (krokujeme přes roky od nejstaršího po nejnovější)
        for (const auto &[year, monthMap]: yearMap) {
            for (const auto &[month, avg]: monthMap) {
                // Pokud na sebe roky přesně navazují (např. 2021 a 2022), provedeme kontrolu
                if (prevMonthData[month].first == year - 1) {
                    const double diff = std::abs(avg - prevMonthData[month].second);

                    // Je-li rozdíl větší než vypočtený threshold, jedná se o anomálii
                    if (diff > thresholds[month]) {
                        result.push_back({stationId, month, year, diff});
                    }
                }
                // Uložíme aktuální rok pro kontrolu v dalším iterovaném cyklu
                prevMonthData[month] = {year, avg};
            }
        }
    }

    return result;
}

/**
 * @brief Paralelně detekuje teplotní anomálie pro všechny stanice.
 *
 * Využívá vícevláknové zpracování k urychlení detekce na velkých datasetech.
 *
 * @param averages Vnořená mapa měsíčních průměrů [ID stanice -> [Rok -> [Měsíc -> Teplota]]].
 *
 * @return std::vector<Anomaly> Seznam detekovaných anomálií seřazený chronologicky a dle stanic.
 */
std::vector<Anomaly> detectAnomaliesParallel(
    const std::map<int, std::map<int, std::map<int, double> > > &averages) {
    // Extrahujeme seznam stanic, abychom mohli mapovat iterátory pro paralelní zpracování
    std::vector<int> stationIds;
    stationIds.reserve(averages.size());
    for (const auto &id: averages | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Vektor, do kterého vlákna uloží své nalezené anomálie (eliminace race conditions)
    std::vector<std::vector<Anomaly> > threadResults(stationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = averages.at(stationId);
        std::vector<Anomaly> localAnomalies;

        std::array<double, 13> minVals{};
        std::array<double, 13> maxVals{};
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        for (const auto &monthMap: yearMap | std::views::values) {
            for (const auto &[month, avg]: monthMap) {
                if (avg < minVals[month]) minVals[month] = avg;
                if (avg > maxVals[month]) maxVals[month] = avg;
            }
        }

        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &p: prevMonthData | std::views::keys) p = -1;

        for (const auto &[year, monthMap]: yearMap) {
            for (const auto &[month, avg]: monthMap) {
                if (prevMonthData[month].first == year - 1) {
                    const double diff = std::abs(avg - prevMonthData[month].second);

                    if (diff > thresholds[month]) {
                        localAnomalies.push_back({stationId, month, year, diff});
                    }
                }
                prevMonthData[month] = {year, avg};
            }
        }

        // Přesun výsledků zpět do vyhrazeného slotu ve společném vektoru
        threadResults[i] = std::move(localAnomalies);
    });

    // Sekvenční sloučení výsledků do jednoho velkého vektoru
    size_t totalAnomalies = 0;
    for (const auto &res: threadResults) {
        totalAnomalies += res.size();
    }

    std::vector<Anomaly> finalAnomalies;
    finalAnomalies.reserve(totalAnomalies);

    // Pomocí move_iterator se vyhneme zbytečnému kopírování nalezených anomálií v paměti
    for (auto &res: threadResults) {
        finalAnomalies.insert(finalAnomalies.end(),
                              std::make_move_iterator(res.begin()),
                              std::make_move_iterator(res.end()));
    }

    // Lambda funkce pro primární/sekundární/terciární kritérium řazení anomálií
    auto byStationYearMonth = [](const Anomaly &a, const Anomaly &b) {
        return std::tie(a.station_id, a.year, a.month) <
               std::tie(b.station_id, b.year, b.month);
    };

    // Dynamický výběr algoritmu řazení na základě velikosti výsledného pole.
    // U menších dat má std::execution::par sort příliš vysokou režii vytvoření vláken.
    constexpr size_t PAR_SORT_THRESHOLD = 200000;
    if (finalAnomalies.size() >= PAR_SORT_THRESHOLD) {
        std::sort(std::execution::par, finalAnomalies.begin(), finalAnomalies.end(), byStationYearMonth);
    } else {
        std::ranges::sort(finalAnomalies, byStationYearMonth);
    }

    return finalAnomalies;
}
