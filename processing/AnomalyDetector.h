#pragma once
#include <map>
#include <vector>

#include "../data/Anomaly.h"

std::vector<Anomaly> detectAnomalies(const std::map<int, std::map<int, std::map<int, double>>> &averages);
