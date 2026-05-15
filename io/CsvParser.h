#pragma once
#include <vector>
#include <string>

#include "../data/Measurement.h"
#include "../data/Station.h"

/**
 * @file CsvParser.h
 *
 * @brief Modul poskytující funkce pro vysoce výkonné parsování CSV souborů.
 *
 * Modul obsahuje synchronní (sériové) i asynchronní (paralelní) varianty načítání
 * dat ze souborů za účelem dosažení maximální propustnosti I/O operací a CPU.
 */

/**
 * @brief Načte seznam meteorologických stanic ze zadaného CSV souboru (sériově).
 *
 * @param path Cesta k CSV souboru obsahujícímu data o stanicích.
 *
 * @return std::vector<Station> Vektor naparsovaných stanic. V případě selhání vrací prázdný vektor.
 */
std::vector<Station> loadStationsSerial(const std::string &path);

/**
 * @brief Načte naměřené hodnoty ze zadaného CSV souboru (sériově).
 *
 * @param path Cesta k CSV souboru obsahujícímu měření.
 *
 * @return std::vector<Measurement> Vektor naparsovaných měření. V případě selhání vrací prázdný vektor.
 */
std::vector<Measurement> loadMeasurementSerial(const std::string &path);

/**
 * @brief Načte naměřené hodnoty ze zadaného CSV souboru s využitím více vláken.
 *
 * Funkce rozdělí soubor na logické bloky a zpracuje je paralelně pomocí dostupných
 * hardwarových vláken, což výrazně zrychluje parsování u rozsáhlých datových sad.
 *
 * @param path Cesta k CSV souboru obsahujícímu měření.
 *
 * @return std::vector<Measurement> Vektor naparsovaných měření. V případě selhání vrací prázdný vektor.
 */
std::vector<Measurement> loadMeasurementParallel(const std::string &path);

/**
 * @brief Načte seznam meteorologických stanic ze zadaného CSV souboru s využitím více vláken.
 *
 * @param path Cesta k CSV souboru obsahujícímu data o stanicích.
 *
 * @return std::vector<Station> Vektor naparsovaných stanic. V případě selhání vrací prázdný vektor.
 */
std::vector<Station> loadStationsParallel(const std::string &path);
