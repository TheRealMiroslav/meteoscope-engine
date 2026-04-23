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

std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages) {
    std::vector<Anomaly> result;

    for (const auto &[stationId, yearMap]: averages) {
        // Lokální MIN a MAX pro tuto stanici
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

        // Předvypočítáme si thresholdy pro měsíce této stanice
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // Chronologická detekce výkyvů
        std::array<std::pair<int, double>, 13> prevMonthData;
        for (auto &fst: prevMonthData | std::views::keys) fst = -1; // -1 = zatím neznámý rok

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
    for (const auto &id: averages | std::views::keys) {
        stationIds.push_back(id);
    }

    std::vector<size_t> indices(stationIds.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<std::vector<Anomaly> > threadResults(stationIds.size());

    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](const size_t i) {
        const int stationId = stationIds[i];
        const auto &yearMap = averages.at(stationId);
        std::vector<Anomaly> localAnomalies;

        // Lokální MIN a MAX pro tuto stanici
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

        // Výpočet hranic výkyvu pro měsíce této stanice
        std::array<double, 13> thresholds{0};
        for (int m = 1; m <= 12; ++m) {
            thresholds[m] = 0.75 * (maxVals[m] - minVals[m]);
        }

        // Chronologická detekce výkyvů
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

        threadResults[i] = std::move(localAnomalies);
    });

    size_t totalAnomalies = 0;
    for (const auto &res: threadResults) {
        totalAnomalies += res.size();
    }

    std::vector<Anomaly> finalAnomalies;
    finalAnomalies.reserve(totalAnomalies);

    for (auto &res: threadResults) {
        finalAnomalies.insert(finalAnomalies.end(),
                              std::make_move_iterator(res.begin()),
                              std::make_move_iterator(res.end()));
    }

    auto byStationYearMonth = [](const Anomaly &a, const Anomaly &b) {
        return std::tie(a.station_id, a.year, a.month) <
               std::tie(b.station_id, b.year, b.month);
    };

    // U menších dat má serial sort nižší režii, větší data umí využít paralelní sort.
    constexpr size_t PAR_SORT_THRESHOLD = 200000;
    if (finalAnomalies.size() >= PAR_SORT_THRESHOLD) {
        std::sort(std::execution::par, finalAnomalies.begin(), finalAnomalies.end(), byStationYearMonth);
    } else {
        std::ranges::sort(finalAnomalies, byStationYearMonth);
    }

    return finalAnomalies;
}
