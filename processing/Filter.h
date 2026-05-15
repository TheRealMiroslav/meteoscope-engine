#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "../data/Measurement.h"

/**
 * @file Filter.h
 * @brief Deklarace funkcí pro filtraci meteorologických stanic na základě kvality a objemu dat.
 */

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
    int minYears,
    int minPerYear);

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
    int minYears, int minPerYear);
