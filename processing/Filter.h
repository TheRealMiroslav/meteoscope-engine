#pragma once
#include <vector>

#include "../data/Measurement.h"
#include "../data/Station.h"

std::vector<int> filterMinYears(const std::vector<Station>& stations, const std::vector<Measurement>& measurements, int minYears);

std::vector<int> filterMinReadings(const std::vector<Station>& stations, const std::vector<Measurement>& measurements, int minPerYear);