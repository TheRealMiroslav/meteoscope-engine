#pragma once
#include <string>
#include <vector>

#include "../data/Anomaly.h"

void writeSerialAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);

void writeParallelAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &filePath);
