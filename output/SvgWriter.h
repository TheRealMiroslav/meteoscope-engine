#pragma once
#include <vector>
#include <map>
#include <string>

#include "../data/Station.h"

/**
 * @file SvgWriter.h
 * @brief Modul pro generování SVG teplotních map na základě naměřených dat.
 *
 * Poskytuje funkce pro sekvenční i paralelní generování mapových výstupů
 * pro jednotlivé měsíce. Do podkladové mapy jsou zakresleny meteorologické stanice
 * jako barevně odlišené body odpovídající průměrné teplotě v daném měsíci.
 */

/**
 * @brief Sekvenčně vygeneruje SVG mapy pro všech 12 měsíců v roce.
 *
 * Funkce načte podkladovou šablonu, a následně pro každý měsíc iteruje přes
 * všechny stanice, vypočítá jejich barvu a souřadnice a výsledné SVG uloží.
 *
 * @param filteredStations Vektor všech dostupných meteorologických stanic.
 * @param monthlyAverages Vnořená mapa obsahující průměrné teploty [stanice -> [rok -> [měsíc -> teplota]]].
 * @param globalMin Celkové teplotní minimum ze všech dat pro správné škálování barev.
 * @param globalMax Celkové teplotní maximum ze všech dat pro správné škálování barev.
 * @param mapSvgPath Cesta k podkladové SVG mapě, která slouží jako šablona.
 * @param outputDir Cílový adresář pro uložení vygenerovaných map.
 */
void writeSvgMapsSerial(const std::vector<Station> &filteredStations,
                        const std::map<int, std::map<int, std::map<int, double> > > &monthlyAverages, double globalMin,
                        double globalMax, const std::string &mapSvgPath, const std::string &outputDir);

/**
 * @brief Paralelně vygeneruje SVG mapy pro všech 12 měsíců v roce.
 *
 * Pro dosažení vyššího výkonu je práce rozdělena pomocí std::execution::par.
 * Každé vlákno řeší jeden měsíc, provádí vlastní výpočty a samostatně zapisuje
 * do vlastního cílového souboru, což zajišťuje bezpečný běh bez nutnosti zámků (lock-free).
 *
 * @param filteredStations Vektor (filtrovaných) meteorologických stanic k vykreslení.
 * @param monthlyAverages Vnořená mapa obsahující průměrné teploty [stanice -> [rok -> [měsíc -> teplota]]].
 * @param globalMin Celkové teplotní minimum ze všech dat pro správné škálování barev.
 * @param globalMax Celkové teplotní maximum ze všech dat pro správné škálování barev.
 * @param mapSvgPath Cesta k podkladové SVG mapě, která slouží jako šablona.
 * @param outputDir Cílový adresář pro uložení vygenerovaných map.
 */
void writeSvgMapsParallel(
    const std::vector<Station> &filteredStations,
    const std::map<int, std::map<int, std::map<int, double> > > &monthlyAverages,
    double globalMin, double globalMax,
    const std::string &mapSvgPath,
    const std::string &outputDir);
