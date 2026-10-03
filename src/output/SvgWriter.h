#pragma once
#include <vector>
#include <map>
#include <string>

#include "../data/Station.h"

/**
 * @file SvgWriter.h
 * @brief Generates SVG temperature heatmaps based on meteorological aggregations.
 *
 * Provides serial and parallel implementations for generating monthly map visualizations.
 * Meteorological observation stations are rendered onto a base vector map template
 * as color-coded circle markers representing monthly mean temperatures.
 */

/**
 * @brief Sequentially generates SVG maps for all 12 calendar months.
 *
 * Reads base SVG template, iterates through stations for each month,
 * evaluates projected coordinates and RGB colors, and writes output files.
 *
 * @param filteredStations Vector of validated meteorological stations.
 * @param monthlyAverages Nested map of mean temperatures [station -> [year -> [month -> temperature]]].
 * @param globalMin Global temperature minimum across entire dataset for color scaling.
 * @param globalMax Global temperature maximum across entire dataset for color scaling.
 * @param mapSvgPath Path to base SVG template file.
 * @param outputDir Target directory for generated maps.
 */
void writeSvgMapsSerial(const std::vector<Station> &filteredStations,
                        const std::map<int, std::map<int, std::map<int, double>>> &monthlyAverages, double globalMin,
                        double globalMax, const std::string &mapSvgPath, const std::string &outputDir);

/**
 * @brief Concurrently generates SVG maps for all 12 calendar months.
 *
 * Distributes month generation across threads using std::execution::par.
 * Each worker operates on dedicated string streams and writes to isolated files (lock-free).
 *
 * @param filteredStations Vector of validated meteorological stations.
 * @param monthlyAverages Nested map of mean temperatures [station -> [year -> [month -> temperature]]].
 * @param globalMin Global temperature minimum across entire dataset for color scaling.
 * @param globalMax Global temperature maximum across entire dataset for color scaling.
 * @param mapSvgPath Path to base SVG template file.
 * @param outputDir Target directory for generated maps.
 */
void writeSvgMapsParallel(const std::vector<Station> &filteredStations,
                          const std::map<int, std::map<int, std::map<int, double>>> &monthlyAverages, double globalMin,
                          double globalMax, const std::string &mapSvgPath, const std::string &outputDir);
