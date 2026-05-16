#pragma once

/**
 * @file ColorMapper.h
 *
 * @brief Modul pro mapování číselných hodnot (teplot) na barevnou škálu.
 */

/**
 * @brief Struktura reprezentující barvu v RGB barevném prostoru.
 */
struct Color {
    int r, g, b;
};

/**
 * @brief Vypočítá RGB barvu na základě hodnoty teploty vzhledem k celkovému minimu a maximu.
 *
 * Barva přechází plynule z modré (nejchladnější) přes zelenou (střed) až po červenou (nejteplejší).
 *
 * @param temp Aktuální teplota, pro kterou chceme získat barvu.
 * @param minTemp Globální minimální teplota (odpovídá čisté modré).
 * @param maxTemp Globální maximální teplota (odpovídá čisté červené).
 *
 * @return Color Struktura obsahující vypočítané složky RGB (0-255).
 */
Color GetColor(double temp, double minTemp, double maxTemp);
