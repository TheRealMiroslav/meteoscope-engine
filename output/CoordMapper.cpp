#include "CoordMapper.h"

#include "../utils/Config.h"

std::pair<int, int> GetCoordinates(const double lat, const double lon) {
    std::pair<int, int> coords;

    const double x = (((lon - Config::LON_MIN) * Config::SVG_WIDTH) / (Config::LON_MAX - Config::LON_MIN));

    const double y = (((Config::LAT_MAX - lat) * Config::SVG_HEIGHT) / (Config::LAT_MAX - Config::LAT_MIN));

    coords.first = static_cast<int>(x);
    coords.second = static_cast<int>(y);

    return coords;
}
