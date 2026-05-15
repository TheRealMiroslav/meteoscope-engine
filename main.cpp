#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

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

    if (mode != "--serial" && mode != "--parallel") {
        std::cerr << "Chyba: Neplatny prepinac '" << mode << "'. Pouzijte --serial nebo --parallel.\n";
        return -1;
    }

    // Načtení dat dle zvoleného režimu (férové měření serial vs parallel)
    std::cout << "Nacitam data ze souboru...\n";
    std::vector<Station> stations;
    std::vector<Measurement> measurements;

    // Vytvoříme a odstartujeme časovač
    Timer timer;
    timer.start();

    if (mode == "--serial") {
        stations = loadStationsSerial(stationPath);
        measurements = loadMeasurementSerial(measurementsPath);

        runSerial(stations, measurements);
    } else {
        stations = loadStationsParallel(stationPath);
        measurements = loadMeasurementParallel(measurementsPath);

        runParallel(stations, measurements);
    }
    std::cout << "Nacitam data ze souboru dokonceno!\n\n";

    /*
    constexpr int runs = 5;
    long soucet = 0;
    for (int i = 0; i < runs; ++i)
        // Spuštění konkrétní logiky podle třetího parametru
        if (mode == "--serial") {
            runSerial(stations, measurements);
        } else if (mode == "--parallel") {
            runParallel(stations, measurements);
        }

        // Zastavíme časovač a vypíšeme výsledek
        timer.stop();
        //std::cout << "========================================\n";
        std::cout << "Celkovy cas zpracovani: " << timer.elapsedSeconds() << " s ("
                << timer.elapsedMilliseconds() << " ms)\n";
        //std::cout << "========================================\n";
        soucet += static_cast<long>(timer.elapsedSeconds() * 1000); // Převod na ms a sčítání
        Sleep(1000);
    }

    soucet /= runs; // Průměrný čas v ms
    std::cout << soucet << std::endl;*/

    // Zastavíme časovač a vypíšeme výsledek
    timer.stop();

    std::cout << "Cas zpracovani: " << timer.elapsedSeconds() << " s ("
                << timer.elapsedMilliseconds() << " ms)\n";

    return 0;
}
