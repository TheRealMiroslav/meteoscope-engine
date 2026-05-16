#pragma once

#include <string>

/**
 * @file Config.h
 *
 * @brief Globální konfigurační konstanty aplikace.
 *
 * Obsahuje geografické hranice, mapovací konstanty pro SVG vizualizace
 * a relativní cesty k výstupním adresářům a souborům.
 */
namespace Config {
    // Hranice mapy
    constexpr double LAT_MAX = 51.03806105663445;
    constexpr double LAT_MIN = 48.521003814763994;
    constexpr double LON_MIN = 12.102209054269062;
    constexpr double LON_MAX = 18.866923511078615;

    // Konfigurace ořezu
    constexpr double PIXEL_X_MIN = 35.0;    // Skutečný začátek mapy zleva (posun)
    constexpr double PIXEL_X_MAX = 1175.0;  // Pravý okraj mapy v pixelech
    constexpr double PIXEL_Y_MIN = 25.0;    // Skutečný začátek mapy shora (posun)
    constexpr double PIXEL_Y_MAX = 585.0;   // Spodní okraj mapy v pixelech

    // Rozměry SVG plátna
    constexpr int SVG_WIDTH = 1412;
    constexpr int SVG_HEIGHT = 809;

    // --- Vizuální nastavení ---
    // Velikost vykresleného bodu reprezentujícího meteorologickou stanici
    constexpr int STATION_RADIUS = 5;

    // --- Cesty k souborům a adresářům ---
    // Relativní cesta k podkladové slepé mapě, na kterou se kreslí data
    constexpr const char *MAP_SVG_PATH = "czmap.svg";
    constexpr const char *OUTPUT_DIR_BASE = "maps/";

    inline const std::string OUTPUT_DIR = OUTPUT_DIR_BASE;
    inline const std::string OUTPUT_SERIAL_MAPS_DIR = OUTPUT_DIR + "serial_maps/";
    inline const std::string OUTPUT_PARALLEL_MAPS_DIR = OUTPUT_DIR + "parallel_maps/";

    inline const std::string OUTPUT_SERIAL_FLUCTUATION_DIR = OUTPUT_DIR + "serial_vykyvy.csv";
    inline const std::string OUTPUT_PARALLEL_FLUCTUATION_DIR = OUTPUT_DIR + "parallel_vykyvy.csv";
}
