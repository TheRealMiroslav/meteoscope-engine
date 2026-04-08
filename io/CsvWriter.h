#pragma once
#include <string>

void writeAnomaliesCsv(anomalies, std::string path);

void writeSvgMaps(stations, averages, globalMin, globalMax, mapSvgPath, outputDir);