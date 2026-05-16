#pragma once
#include <map>
#include <vector>

#include "../data/Anomaly.h"

/**
 * @file AnomalyDetector.h
 * @brief Modul pro detekci teplotních anomálií a výkyvů.
 *
 * Slouží k analýze historických teplotních průměrů a identifikaci
 * signifikantních meziročních výkyvů (např. extrémní skoky průměrné
 * teploty v rámci shodného měsíce u dvou po sobě jdoucích let).
 */

/**
 * @brief Sekvenčně detekuje teplotní anomálie pro všechny stanice.
 *
 * @param averages Vnořená mapa měsíčních průměrů [ID stanice -> [Rok -> [Měsíc -> Teplota]]].
 *
 * @return std::vector<Anomaly> Seznam detekovaných anomálií seřazený chronologicky a dle stanic.
 */
std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double> > > &averages);

/**
 * @brief Paralelně detekuje teplotní anomálie pro všechny stanice.
 *
 * Využívá vícevláknové zpracování k urychlení detekce na velkých datasetech.
 *
 * @param averages Vnořená mapa měsíčních průměrů [ID stanice -> [Rok -> [Měsíc -> Teplota]]].
 *
 * @return std::vector<Anomaly> Seznam detekovaných anomálií seřazený chronologicky a dle stanic.
 */
std::vector<Anomaly> detectAnomaliesParallel(const std::map<int, std::map<int, std::map<int, double> > > &averages);
