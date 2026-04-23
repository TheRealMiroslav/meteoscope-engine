#pragma once
#include <vector>

#include "../data/Measurement.h"
#include "../data/Station.h"

std::vector<Station> loadStations(const std::string& path);

std::vector<Measurement> loadMeasurement(const std::string& path);

std::vector<Station> loadStationsOptimized(const std::string &path);

std::vector<Measurement> loadMeasurementOptimized(const std::string &path);

std::vector<Measurement> loadMeasurementParallel(const std::string &path);

std::vector<Station> loadStationsParallel(const std::string &path);