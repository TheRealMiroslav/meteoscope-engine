#pragma once
#include <utility>

/**
 * @file CoordMapper.h
 *
 * @brief Converts geographic WGS84 coordinates into 2D SVG canvas pixel coordinates.
 */

/**
 * @brief Projects latitude and longitude into X and Y pixel coordinates for SVG plotting.
 *
 * Uses calibrated boundaries and dimensions defined in Config.
 *
 * @param lat Latitude in decimal degrees.
 * @param lon Longitude in decimal degrees.
 *
 * @return std::pair<int, int> Pair of X and Y coordinates in pixels.
 */
std::pair<int, int> GetCoordinates(double lat, double lon);
