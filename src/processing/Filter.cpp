#include "Filter.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <ranges>
#include <execution>
#include <unordered_map>
#include <numeric>

/**
 * @brief Sériová filtrace stanic na základě minimálních požadavků na data.
 *
 * Funkce vyřadí stanice, které nemají dostatečnou historii měření (nepřerušená řada let)
 * nebo nemají dostatečnou hustotu měření v zaznamenaných letech.
 *
 * @param groupedMeasurements Hierarchická struktura dat: ID stanice -> Rok -> Seznam měření.
 * @param minYears Minimální počet po sobě jdoucích let měření nutný pro zachování stanice.
 * @param minPerYear Minimální průměrný počet měření na jeden zaznamenaný rok.
 *
 * @return std::vector<int> Vektor ID stanic, které splnily kritéria filtrace.
 */
std::vector<int> filterStationsSerial(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minYears,
    const int minPerYear) {
    std::vector<int> result;
    // Heuristická předalokace paměti (odhadujeme, že projde cca 75 % stanic)
    // pro minimalizaci realokací během přidávání do vektoru.
    result.reserve((groupedMeasurements.size() / 4) * 3);

    for (const auto &[stationId, yearMap]: groupedMeasurements) {
        if (yearMap.empty())
            continue;

        // Krok 1: Kontrola hustoty měření
        // Spočítáme celkový počet měření pro danou stanici napříč všemi roky
        size_t totalMeasurements = 0;
        for (const auto &ms: yearMap | std::views::values) {
            totalMeasurements += ms.size();
        }

        // Pokud je průměrný počet měření na aktivní rok menší než limit, stanici vyřadíme
        if (static_cast<int>(totalMeasurements / yearMap.size()) < minPerYear)
            continue;

        // Krok 2: Kontrola souvislosti (po sobě jdoucí roky)
        int lastYear = 0;
        int counter = 0;
        bool passedYears = false;

        // Protože yearMap je std::map, iterace přes klíče je zaručeně chronologická (vzestupně)
        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passedYears = true;
                break; // Limit splněn, můžeme přeskočit kontrolu zbývajících let
            }
        }

        if (passedYears) {
            result.push_back(stationId);
        }
    }

    return result;
}

/**
 * @brief Paralelní filtrace stanic na základě minimálních požadavků na data.
 *
 * Vícevláknová alternativa k `filterStationsSerial`. Využívá `std::execution::par`
 * a lock-free strategii zápisu výsledků pomocí pomocného boolean (int) vektoru.
 *
 * @param groupedMeasurements Hierarchická struktura dat: ID stanice -> Rok -> Seznam měření.
 * @param minYears Minimální počet po sobě jdoucích let měření nutný pro zachování stanice.
 * @param minPerYear Minimální průměrný počet měření na jeden zaznamenaný rok.
 *
 * @return std::vector<int> Vektor ID stanic, které splnily kritéria filtrace.
 */
std::vector<int> filterStationsParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const int minYears,
    const int minPerYear) {
    // Extrakce ID všech dostupných stanic pro rovnoměrné rozdělení práce mezi vlákna
    std::vector<int> stationIds;
    stationIds.reserve(groupedMeasurements.size());
    for (const auto &id: groupedMeasurements | std::views::keys) {
        stationIds.push_back(id);
    }

    // Indikační pole pro výsledky filtrace.
    // Lock-free přístup: každé vlákno bude zapisovat pouze na "svůj" vyhrazený index.
    std::vector<int> passed(stationIds.size(), 0);
    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Paralelní vyhodnocení podmínek pro každou stanici
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = groupedMeasurements.at(stationId);

        if (yearMap.empty()) return;

        // Krok 1: Kontrola hustoty měření (lokální pro dané vlákno)
        size_t totalMeasurements = 0;
        for (const auto &ms: yearMap | std::views::values) {
            totalMeasurements += ms.size();
        }

        if (static_cast<int>(totalMeasurements / yearMap.size()) < minPerYear) {
            return; // Filtrem neprošlo, ve vektoru 'passed' zůstává 0
        }

        // Krok 2: Kontrola kontinuity měření
        int lastYear = 0;
        int counter = 0;
        bool passedYears = false;

        for (const auto &year: yearMap | std::views::keys) {
            counter = (year == lastYear + 1) ? counter + 1 : 1;
            lastYear = year;

            if (counter >= minYears) {
                passedYears = true;
                break;
            }
        }

        if (passedYears) {
            passed[i] = 1; // Zápis výsledku (bez nutnosti std::mutex díky izolovaným indexům)
        }
    });

    // Sekvenční redukce výsledků: shromáždění ID stanic, které dostaly příznak 1
    std::vector<int> result;
    result.reserve((stationIds.size() / 4) * 3);

    for (size_t i = 0; i < stationIds.size(); i++) {
        if (passed[i]) {
            result.push_back(stationIds[i]);
        }
    }

    return result;
}
