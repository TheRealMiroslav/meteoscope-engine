#pragma once

/**
 * @brief Reprezentuje detekovanou anomálii v naměřených datech.
 *
 * Uchovává informace o tom, na jaké stanici, v jakém čase k výkyvu došlo,
 * a jak výrazná tato odchylka byla.
 */
struct Anomaly {
    int station_id; ///< Unikátní identifikátor meteorologické stanice, kde anomálie nastala.
    int month;      ///< Měsíc výskytu anomálie (1-12).
    int year;       ///< Rok výskytu anomálie.
    double diff;    ///< Velikost odchylky (např. teplotní rozdíl oproti očekávanému průměru).
};
