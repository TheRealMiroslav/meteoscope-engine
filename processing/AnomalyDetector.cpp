#include "AnomalyDetector.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>
#include <limits>
#include <tuple>

std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages) {
    std::vector<Anomaly> result;

    for (const auto &[stationId, yearMap]: averages) {

        // 1. Průchod: Najdeme lokální MIN a MAX pro každý měsíc [1..12] pro tuto konkrétní stanici
        std::array<double, 13> minVals;
        std::array<double, 13> maxVals;
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        for (const auto &[year, monthMap]: yearMap) {
            for (const auto &[month, avg]: monthMap) {
                if (avg < minVals[month]) minVals[month] = avg;
                if (avg > maxVals[month]) maxVals[month] = avg;
            }
        }

        // 2. Předvypočítáme si thresholdy pro měsíce této stanice
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // 3. Průchod: Zkontrolujeme anomálie.
        // Protože std::map má roky už automaticky seřazené, stačí si jen pamatovat hodnotu z předchozího roku.
        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &p : prevMonthData) p.first = -1; // -1 = zatím neznámý rok

        for (const auto &[year, monthMap]: yearMap) {
            for (const auto &[month, avg]: monthMap) {
                // Pokud na sebe roky přesně navazují, provedeme kontrolu
                if (prevMonthData[month].first == year - 1) {
                    const double diff = std::abs(avg - prevMonthData[month].second);
                    if (diff > thresholds[month]) {
                        result.push_back({stationId, month, year, diff});
                    }
                }
                // Uložíme aktuální rok pro kontrolu v dalším kole
                prevMonthData[month] = {year, avg};
            }
        }
    }

    return result;
}

std::vector<Anomaly> detectAnomaliesParallel(
    const std::map<int, std::map<int, std::map<int, double> > > &averages) {

    std::vector<int> stationIds;
    stationIds.reserve(averages.size());
    for (auto const &[id, _]: averages) {
        stationIds.push_back(id);
    }

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::vector<Anomaly> > threadResults(stationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = averages.at(stationId);
        std::vector<Anomaly> localAnomalies;

        // 1. Lokální MIN a MAX (bez alokace polí)
        std::array<double, 13> minVals;
        std::array<double, 13> maxVals;
        minVals.fill(std::numeric_limits<double>::max());
        maxVals.fill(std::numeric_limits<double>::lowest());

        for (const auto &[year, monthMap]: yearMap) {
            for (const auto &[month, avg]: monthMap) {
                if (avg < minVals[month]) minVals[month] = avg;
                if (avg > maxVals[month]) maxVals[month] = avg;
            }
        }

        // 2. Výpočet hranic výkyvu (thresholdů)
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // 3. Chronologická detekce výkyvů
        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &p : prevMonthData) p.first = -1;

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

        threadResults[i] = std::move(localAnomalies);
    });

    // Sériový sběr výsledků vláken
    std::vector<Anomaly> finalAnomalies;
    for (const auto &res: threadResults) {
        finalAnomalies.insert(finalAnomalies.end(), res.begin(), res.end());
    }

    std::ranges::sort(finalAnomalies, [](const Anomaly &a, const Anomaly &b) {
        return std::tie(a.station_id, a.year, a.month) <
               std::tie(b.station_id, b.month, b.year);
    });

    return finalAnomalies;
}