#pragma once

#include <vector>

#include "../data/Station.h"
#include "../data/Measurement.h"

/**
 * @brief Sériová verze hlavního procesu pro zpracování meteorologických dat.
 *
 * Funkce postupně seskupí data, vyfiltruje validní stanice (podle počtu let a záznamů),
 * vypočítá měsíční průměry, najde globální extrémy, detekuje anomálie a následně
 * vygeneruje CSV reporty a SVG mapy. Vše probíhá sekvenčně v jednom vlákně.
 *
 * @param stations Vektor všech dostupných meteorologických stanic.
 * @param measurements Vektor všech naměřených hodnot ke zpracování.
 */
void runSerial(const std::vector<Station> &stations, const std::vector<Measurement> &measurements);

/**
 * @brief Paralelní verze hlavního procesu pro zpracování meteorologických dat.
 *
 * Funkce provádí stejné kroky jako `runSerial`, ale využívá vícevláknové zpracování
 * (std::thread, std::execution::par) k optimalizaci výpočtů a agregace dat.
 * Obsahuje map-reduce logiku pro bezpečné rozdělení a sloučení dat mezi vlákny.
 *
 * @param stations Vektor všech dostupných meteorologických stanic.
 * @param measurements Vektor všech naměřených hodnot ke zpracování.
 */
void runParallel(const std::vector<Station> &stations, const std::vector<Measurement> &measurements);
