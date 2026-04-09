#pragma once

#include <vector>

#include "../data/Station.h"
#include "../data/Measurement.h"

void runSerial(const std::vector<Station>& stations, const std::vector<Measurement>& measurements);

void runParallel(const std::vector<Station>& stations, const std::vector<Measurement>& measurements);