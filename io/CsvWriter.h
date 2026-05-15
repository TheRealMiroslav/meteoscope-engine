#pragma once
#include <string>
#include <vector>

#include "../data/Anomaly.h"

/**
 * @brief Sériový zápis nalezených anomálií do CSV formátu.
 *
 * @param anomalies Vektor detekovaných anomálií k exportu.
 * @param filePath Cesta, kam se má výsledný soubor uložit.
 *
 * @throws std::runtime_error Pokud nelze cílový soubor otevřít pro zápis.
 */
void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);

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
void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);
