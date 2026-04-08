#pragma once
#include <vector>
#include <map>
#include <string>

#include "../data/Station.h"

void writeSvgMaps(const std::vector<Station> &stations,
                  const std::map<int, std::map<int, std::map<int, double>> > &averages, double globalMin,
                  double globalMax, const std::string &mapSvgPath, const std::string &outputDir);

double getStationMonthAverage(
    const std::map<int, std::map<int, std::map<int, double> > > &averages,
    int stationId, int month);
