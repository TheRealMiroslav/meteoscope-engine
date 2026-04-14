#pragma once

#include <string>

namespace Config {
    // Hranice mapy
    constexpr double LAT_MAX = 51.03806105663445;
    constexpr double LAT_MIN = 48.521003814763994;
    constexpr double LON_MIN = 12.102209054269062;
    constexpr double LON_MAX = 18.866923511078615;

    constexpr double PIXEL_X_MIN = 35.0;   // O kolik pixelů je mapa posunutá zleva
    constexpr double PIXEL_X_MAX = 1175.0; // Kde mapa reálně končí vpravo
    constexpr double PIXEL_Y_MIN = 25.0;   // Posun shora
    constexpr double PIXEL_Y_MAX = 585.0;  // Kde končí dole

    // SVG rozměry
    constexpr int SVG_WIDTH = 1412;
    constexpr int SVG_HEIGHT = 809;

    // Velikost bodu stanice
    constexpr int STATION_RADIUS = 8;

    // Cesta k slepé mapě
    constexpr auto MAP_SVG_PATH = "czmap.svg";

    constexpr std::string OUTPUT_DIR = "../maps/";

    const std::string OUTPUT_SERIAL_MAPS_DIR = OUTPUT_DIR + "serial_maps/";
    const std::string OUTPUT_PARALLEL_MAPS_DIR = OUTPUT_DIR + "parallel_maps/";

    const std::string OUTPUT_SERIAL_FLUCTUATION_DIR = OUTPUT_DIR + "serial_vykyvy.csv";
    const std::string OUTPUT_PARALLEL_FLUCTUATION_DIR = OUTPUT_DIR + "parallel_vykyvy.csv";
}
