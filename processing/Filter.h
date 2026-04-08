#pragma once
#include <map>
#include <vector>

#include "../data/Measurement.h"
#include "../data/Station.h"

std::vector<int> filterMinYears(const std::map<int, std::map<int, std::vector<Measurement>>>& groupedMeasurements, int minYears);

std::vector<int> filterMinReadings(const std::map<int, std::map<int, std::vector<Measurement>>>& groupedMeasurements, int minPerYear);