#pragma once
#include <utility>

/**
 * @file CoordMapper.h
 *
 * @brief Modul pro převod geografických souřadnic (WGS84) na souřadnice 2D plátna (SVG).
 */

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
std::pair<int, int> GetCoordinates(double lat, double lon);
