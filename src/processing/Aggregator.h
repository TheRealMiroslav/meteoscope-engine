#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "../data/Measurement.h"

/**
 * @file Aggregator.h
 * @brief Modul pro agregaci naměřených meteorologických dat.
 *
 * Poskytuje funkce pro výpočet průměrných měsíčních teplot z denních měření
 * pro specifikované stanice. K dispozici je sekvenční i paralelní varianta.
 */

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
    const std::vector<int> &passedStationIds);

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
    const std::vector<int> &passedStationIds);


