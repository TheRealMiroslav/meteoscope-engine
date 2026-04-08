#pragma once
#include <vector>

#include "../data/Measurement.h"
#include "../data/Station.h"

std::vector<Station> loadStations(const std::string& path);

std::vector<Measurement> loadMeasurement(const std::string& path);