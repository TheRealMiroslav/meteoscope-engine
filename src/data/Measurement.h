#pragma once

/**
 * @brief Datová struktura reprezentující jeden konkrétní záznam měření.
 *
 * Slouží k uchování surových dat načtených ze vstupních souborů před jejich agregací.
 */
struct Measurement {
    int id;         ///< Identifikátor stanice, ke které toto měření patří.
    int ordinal;    ///< Pořadové číslo měření (např. den v roce nebo absolutní index záznamu).
    int year;       ///< Rok, kdy bylo měření provedeno.
    int month;      ///< Měsíc, kdy bylo měření provedeno (1-12).
    float value;    ///< Samotná naměřená hodnota (např. průměrná denní teplota).
};