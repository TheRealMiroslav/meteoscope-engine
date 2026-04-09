#include <iostream>
#include <string>
#include <windows.h>

#include "data/Station.h"
#include "io/CsvParser.h"
#include "core/AppRunner.h"
#include "utils/Timer.h"

int main(const int argc, char const *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Kontrola argumentů přesně podle zadání
    if (argc != 4) {
        std::cerr << "Pouziti: " << argv[0] << " <stanice.csv> <mereni.csv> <--serial|--parallel>\n";
        return -1;
    }

    const std::string stationPath = argv[1];
    const std::string measurementsPath = argv[2];
    const std::string mode = argv[3];

    // Načtení dat
    std::cout << "Nacitam data ze souboru...\n";
    const std::vector<Station> stations = loadStations(stationPath);
    const std::vector<Measurement> measurements = loadMeasurement(measurementsPath);
    std::cout << "Nacitam data ze souboru dokonceno!\n\n";

    // Vytvoříme a odstartujeme časovač
    Timer timer;
    timer.start();

    // Spuštění konkrétní logiky podle třetího parametru
    if (mode == "--serial") {
        runSerial(stations, measurements);
    } else if (mode == "--parallel") {
        runParallel(stations, measurements);
    } else {
        std::cerr << "Chyba: Neplatny prepinac '" << mode << "'. Pouzijte --serial nebo --parallel.\n";
        return -1;
    }

    // Zastavíme časovač a vypíšeme výsledek
    timer.stop();
    std::cout << "========================================\n";
    std::cout << "Celkovy cas zpracovani: " << timer.elapsedSeconds() << " s ("
            << timer.elapsedMilliseconds() << " ms)\n";
    std::cout << "========================================\n";

    return 0;
}
