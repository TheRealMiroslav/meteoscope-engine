#include "CsvWriter.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <numeric>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Sériový zápis nalezených anomálií do CSV formátu.
 *
 * @param anomalies Vektor detekovaných anomálií k exportu.
 * @param filePath Cesta, kam se má výsledný soubor uložit.
 *
 * @throws std::runtime_error Pokud nelze cílový soubor otevřít pro zápis.
 */
void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    // Automatické vytvoření výstupní složky, pokud neexistuje
    fs::path path(filePath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se vytvorit soubor pro zapis anomalii: " + filePath);
    }

    file << "station_id;month;year;diff\n";

    for (const auto &[station_id, month, year, diff]: anomalies) {
        // Přímý zápis dat do streamu.
        // Iostream automaticky formátuje číselné typy, což je bezpečné,
        // ale v masivních iteracích to může být pomalejší.
        file << station_id << ";" << month << ";" << year << ";" << diff << "\n";
    }

    file.close();
}

/**
 * @brief Paralelizovaný zápis nalezených anomálií do CSV formátu.
 *
 * Odděluje na CPU náročné formátování textu (provádí se vícevláknově)
 * od samotného diskového I/O zápisu (provádí se sériově pro zamezení race conditions na disku).
 *
 * @param anomalies Vektor detekovaných anomálií k exportu.
 * @param filePath Cesta, kam se má výsledný soubor uložit.
 *
 * @throws std::runtime_error Pokud nelze cílový soubor otevřít pro zápis.
 */
void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    // Automatické vytvoření výstupní složky, pokud neexistuje
    fs::path path(filePath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se vytvorit soubor pro zapis anomalii: " + filePath);
    }

    // 1. Zápis hlavičky (sériově)
    file << "station_id;month;year;diff\n";

    if (anomalies.empty()) {
        return; // Pokud nejsou data, nemá smysl alokovat vektory pro texty
    }

    // 2. Pre-alokace pole pro textové řádky
    // Každá anomálie dostane svůj slot ve vektoru typu string
    std::vector<std::string> lines(anomalies.size());
    std::vector<size_t> indices(anomalies.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 3. Paralelní formátování dat
    // Převod čísel na text (formatting) je výpočetně drahá operace.
    // Zde ji provádíme paralelně do předem alokovaného pole řetězců.
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const auto &[station_id, month, year, diff] = anomalies[i];

        char buffer[128];

        // Využití snprintf z jazyka C je často efektivnější než std::to_string s následnou konkatenací (+),
        // protože negeneruje tolik dočasných objektů v paměti.
        std::snprintf(buffer, sizeof(buffer), "%d;%d;%d;%g\n", station_id, month, year, diff);

        lines[i] = std::string(buffer);
    });

    // 4. Bleskový sériový zápis hotových textů do souboru
    // I/O operace do jednoho souboru nelze spolehlivě paralelizovat (hrozí promíchání dat),
    // proto samotný flush na disk musí proběhnout v jednom vlákně.
    for (const auto &line: lines) {
        file << line;
    }

    file.close();
}
