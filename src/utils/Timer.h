#pragma once

#include <chrono>

/**
 * @file Timer.h
 *
 * @brief Pomocná třída pro vysoce přesné měření času provádění kódu.
 *
 * Využívá std::chrono::high_resolution_clock pro přesné profilování
 * sériových a paralelních algoritmů.
 */
class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop_time;
    bool running = false;

public:
    /**
     * @brief Konstruktor, který automaticky spustí odpočet.
     */
    Timer() {
        start();
    }

    /**
     * @brief (Znovu)spustí nebo zresetuje časovač.
     */
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    /**
     * @brief Zastaví časovač a uloží koncový čas.
     */
    void stop() {
        stop_time = std::chrono::high_resolution_clock::now();
        running = false;
    }

    /**
     * @brief Vrací uplynulý čas v sekundách.
     *
     * Pokud časovač stále běží, vrací čas od spuštění do aktuálního okamžiku.
     * Pokud byl zastaven, vrací rozdíl mezi startem a zastavením.
     *
     * @return Uplynulý čas v sekundách (desetinné číslo).
     */
    [[nodiscard]] double elapsedSeconds() const {
        const auto end_time = running ? std::chrono::high_resolution_clock::now() : stop_time;
        const std::chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }

    /**
     * @brief Vrací uplynulý čas v milisekundách.
     *
     * @return Uplynulý čas v milisekundách (desetinné číslo).
     */
    [[nodiscard]] double elapsedMilliseconds() const {
        return elapsedSeconds() * 1000.0;
    }
};
