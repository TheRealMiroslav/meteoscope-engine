#include "CsvWriter.h"
#include <fstream>


void writeAnomaliesCsv(const std::vector<Anomaly> &anomalies, const std::string &path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        // zaloguj chybu nebo vyhoď výjimku
    }

    file << "station_id;month;year;diff\n";

    for (const auto &[station_id, month, year, diff] : anomalies) {
        file << station_id << ";" << month << ";" << year << ";" << diff << std::endl;
    }

    file.close();
}