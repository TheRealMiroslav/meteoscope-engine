#pragma once
#include <map>
#include <vector>

#include "../data/Measurement.h"

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds);


