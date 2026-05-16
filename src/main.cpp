/**
 * @file main.cpp
 *
 * @brief Hlavní vstupní bod aplikace pro zpracování meteorologických dat.
 *
 * Zajišťuje zpracování parametrů příkazové řádky, inicializaci, načtení dat
 * (sériově či paralelně) a spuštění příslušné výpočetní logiky pro vyhodnocení
 * a generování výstupů (CSV, SVG).
 */

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "data/Station.h"
#include "io/CsvParser.h"
#include "core/AppRunner.h"
#include "utils/Timer.h"

/**
 * @brief Hlavní funkce programu.
 *
 * Očekává přesně 3 parametry příkazové řádky:
 * 1. Cestu k CSV souboru se stanicemi
 * 2. Cestu k CSV souboru s měřeními
 * 3. Přepínač režimu zpracování (--serial nebo --parallel)
 *
 * @param argc Počet argumentů zadaných z příkazové řádky.
 * @param argv Pole textových řetězců reprezentujících argumenty.
 * @return Nula při úspěšném běhu, -1 v případě chyby vstupních parametrů.
 */
int main(const int argc, char const *argv[]) {
#ifdef _WIN32
    // Nastavení kódování konzole na UTF-8 pro správné zobrazení (i případné diakritiky)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Kontrola počtu argumentů
    if (argc != 4) {
        std::cerr << "Pouziti: " << argv[0] << " <stanice.csv> <mereni.csv> <--serial|--parallel>\n";
        return -1;
    }

    const std::string stationPath = argv[1];
    const std::string measurementsPath = argv[2];
    const std::string mode = argv[3];

    // Validace zvoleného režimu zpracování
    if (mode != "--serial" && mode != "--parallel") {
        std::cerr << "Chyba: Neplatny prepinac '" << mode << "'. Pouzijte --serial nebo --parallel.\n";
        return -1;
    }

    // Příprava datových struktur
    std::vector<Station> stations;
    std::vector<Measurement> measurements;

    // --- ZAČÁTEK MĚŘENÉHO BLOKU ---
    // Timer se v konstruktoru automaticky spustí
    Timer totalTimer;

    // 1. Fáze: Načítání
    Timer loadTimer;
    if (mode == "--serial") {
        stations = loadStationsSerial(stationPath);
        measurements = loadMeasurementSerial(measurementsPath);
    } else {
        stations = loadStationsParallel(stationPath);
        measurements = loadMeasurementParallel(measurementsPath);
    }
    loadTimer.stop();

    // --- KONTROLA NAČTENÝCH DAT ---
    // Pokud se nepodařilo načíst žádná data, vypíšeme chybu a ukončíme program
    if (stations.empty()) {
        std::cerr << "Chyba: Nepodarilo se nacist zadne stanice ze souboru '" << stationPath << "'. Soubor neexistuje nebo je prazdny.\n";
        return -1;
    }

    if (measurements.empty()) {
        std::cerr << "Chyba: Nepodarilo se nacist zadna mereni ze souboru '" << measurementsPath << "'. Soubor neexistuje nebo je prazdny.\n";
        return -1;
    }

    // 2. Fáze: Zpracování (výpočty, detekce anomálií, zápis souborů)
    Timer processTimer;
    if (mode == "--serial") {
        runSerial(stations, measurements);
    } else {
        runParallel(stations, measurements);
    }
    processTimer.stop();

    totalTimer.stop();
    // --- KONEC MĚŘENÉHO BLOKU ---

    // Veškeré výpisy probíhají až zde, kdy už jsou stopky zastaveny
    std::cout << "================================================\n";
    std::cout << "       METEOROLOGICKA ANALYZA DOKONCENA         \n";
    std::cout << "================================================\n";
    std::cout << " Rezim:            " << (mode == "--parallel" ? "PARALELNI" : "SERIOVY") << "\n";
    std::cout << " Pocet stanic:     " << stations.size() << "\n";
    std::cout << " Pocet mereni:     " << measurements.size() << "\n";
    std::cout << "------------------------------------------------\n";

    // Nastavení formátu desetinných čísel
    std::cout << std::fixed << std::setprecision(3);

    std::cout << " Cas nacitani:     " << loadTimer.elapsedSeconds() << " s\n";
    std::cout << " Cas zpracovani:   " << processTimer.elapsedSeconds() << " s\n";
    std::cout << "------------------------------------------------\n";
    std::cout << " CELKOVY CAS:      " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << "================================================\n";

    return 0;
}
