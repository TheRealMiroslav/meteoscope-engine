#pragma once
#include <map>
#include <unordered_map>
#include <vector>

#include "../data/Measurement.h"

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAverages(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds);

std::map<int, std::map<int, std::map<int, double> > > computeMonthlyAveragesParallel(
    const std::unordered_map<int, std::map<int, std::vector<Measurement> > > &groupedMeasurements,
    const std::vector<int> &passedStationIds);


