#pragma once
#include <string>
#include <vector>

#include "../data/Anomaly.h"

void writeAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &path);

//void writeSvgMaps(stations, averages, globalMin, globalMax, mapSvgPath, outputDir);