#include "CoordMapper.h"
#include "../utils/Config.h"

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
std::pair<int, int> GetCoordinates(const double lat, const double lon) {
    std::pair<int, int> coords;

    // Linear projection of longitude onto X axis (left to right)
    const double x = (((lon - Config::LON_MIN) * Config::SVG_WIDTH) / (Config::LON_MAX - Config::LON_MIN));

    // Linear projection of latitude onto Y axis
    // In SVG space, Y increases downwards, hence inversion relative to equator
    const double y = (((Config::LAT_MAX - lat) * Config::SVG_HEIGHT) / (Config::LAT_MAX - Config::LAT_MIN));

    coords.first = static_cast<int>(x);
    coords.second = static_cast<int>(y);

    return coords;
}
