#include "CoordMapper.h"
#include "../utils/Config.h"

/**
 * @brief Převede zeměpisnou šířku a délku na souřadnice X a Y pro vykreslení v SVG.
 *
 * Využívá konstanty definované ve třídě Config pro určení rozměrů plátna a okrajů bounding boxu.
 *
 * @param lat Zeměpisná šířka (Latitude).
 * @param lon Zeměpisná délka (Longitude).
 *
 * @return std::pair<int, int> Dvojice X a Y souřadnic v pixelech.
 */
std::pair<int, int> GetCoordinates(const double lat, const double lon) {
    std::pair<int, int> coords;

    // Lineární transformace zeměpisné délky (Longitude) na osu X zleva doprava.
    const double x = (((lon - Config::LON_MIN) * Config::SVG_WIDTH) / (Config::LON_MAX - Config::LON_MIN));

    // Lineární transformace zeměpisné šířky (Latitude) na osu Y.
    // V SVG roste souřadnice Y směrem dolů, proto je výpočet invertován vzhledem k rovníku.
    const double y = (((Config::LAT_MAX - lat) * Config::SVG_HEIGHT) / (Config::LAT_MAX - Config::LAT_MIN));

    coords.first = static_cast<int>(x);
    coords.second = static_cast<int>(y);

    return coords;
}
