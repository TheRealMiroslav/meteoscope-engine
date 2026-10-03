#pragma once

#include <string>

/**
 * @file Config.h
 *
 * @brief Global application configuration constants.
 *
 * Defines geographic coordinate bounds, projection parameters for SVG
 * vector visualizations, and paths for output directories and anomaly reports.
 */
namespace Config {
    // Geographic Projection Bounds (Czech Republic coordinate bounding box)
    constexpr double LAT_MAX = 51.03806105663445;
    constexpr double LAT_MIN = 48.521003814763994;
    constexpr double LON_MIN = 12.102209054269062;
    constexpr double LON_MAX = 18.866923511078615;

    // Canvas Calibration Configuration
    constexpr double PIXEL_X_MIN = 35.0;    // Left margin offset
    constexpr double PIXEL_X_MAX = 1175.0;  // Right canvas boundary in pixels
    constexpr double PIXEL_Y_MIN = 25.0;    // Top margin offset
    constexpr double PIXEL_Y_MAX = 585.0;   // Bottom canvas boundary in pixels

    // SVG Canvas Dimensions
    constexpr int SVG_WIDTH = 1412;
    constexpr int SVG_HEIGHT = 809;

    // Visual Settings
    // Radius of rendered circle representing a meteorological station
    constexpr int STATION_RADIUS = 5;

    // File and Directory Paths
    // Relative path to base map template
    constexpr const char *MAP_SVG_PATH = "czmap.svg";
    constexpr const char *OUTPUT_DIR_BASE = "maps/";

    inline const std::string OUTPUT_DIR = OUTPUT_DIR_BASE;
    inline const std::string OUTPUT_SERIAL_MAPS_DIR = OUTPUT_DIR + "serial_maps/";
    inline const std::string OUTPUT_PARALLEL_MAPS_DIR = OUTPUT_DIR + "parallel_maps/";

    inline const std::string OUTPUT_SERIAL_FLUCTUATION_DIR = OUTPUT_DIR + "serial_anomalies.csv";
    inline const std::string OUTPUT_PARALLEL_FLUCTUATION_DIR = OUTPUT_DIR + "parallel_anomalies.csv";
}
