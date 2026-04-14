#include "CsvWriter.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <numeric>
#include <sstream>


void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se vytvorit soubor pro zapis anomalii: " + filePath);
    }

    file << "station_id;month;year;diff\n";

    for (const auto &a: anomalies) {
        // Změněno na přímý přístup k atributům kvůli konzistenci
        file << a.station_id << ";" << a.month << ";" << a.year << ";" << a.diff << "\n";
    }

    file.close();
}

void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Chyba: Nepodarilo se vytvorit soubor pro zapis anomalii: " + filePath);
    }

    // 1. Zápis hlavičky (sériově)
    file << "station_id;month;year;diff\n";

    if (anomalies.empty()) {
        return; // Pokud nejsou data, nemá smysl alokovat vektory
    }

    // 2. Pre-alokace pole pro textové řádky
    std::vector<std::string> lines(anomalies.size());
    std::vector<size_t> indices(anomalies.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 3. Paralelní formátování dat (převod čísel na text je CPU-heavy)
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        const auto &a = anomalies[i];
        std::ostringstream oss;
        oss << a.station_id << ";" << a.month << ";" << a.year << ";" << a.diff << "\n";
        lines[i] = oss.str();
    });

    // 4. Bleskový sériový zápis hotových textů do souboru
    for (const auto &line: lines) {
        file << line;
    }

    file.close();
}
