#pragma once

/**
 * @brief Reprezentuje meteorologickou stanici a její geografickou polohu.
 *
 * Slouží primárně k mapování naměřených dat na konkrétní fyzické umístění,
 * což je nezbytné pro následnou vizualizaci (např. SVG mapy).
 */
struct Station {
    int id;         ///< Unikátní identifikátor meteorologické stanice.
    double lat;     ///< Zeměpisná šířka (latitude) polohy stanice.
    double lon;     ///< Zeměpisná délka (longitude) polohy stanice.
};