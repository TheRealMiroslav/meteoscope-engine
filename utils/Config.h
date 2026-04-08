#pragma once

namespace Config {
    // Hranice mapy
    constexpr double LAT_MAX = 51.03806105663445;
    constexpr double LAT_MIN = 48.521003814763994;
    constexpr double LON_MIN = 12.102209054269062;
    constexpr double LON_MAX = 18.866923511078615;

    // SVG rozměry
    constexpr int SVG_WIDTH  = 1200;
    constexpr int SVG_HEIGHT = 600;

    // Velikost bodu stanice
    constexpr int STATION_RADIUS = 8;

    // Cesta k slepé mapě
    constexpr auto MAP_SVG_PATH = "czmap.svg";
}