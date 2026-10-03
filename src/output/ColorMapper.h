#pragma once

/**
 * @file ColorMapper.h
 *
 * @brief Maps scalar metrics (temperatures) to continuous color gradients.
 */

/**
 * @brief Structure representing a color in RGB space.
 */
struct Color {
    int r, g, b;
};

/**
 * @brief Computes RGB color interpolated across a thermal scale bounded by min and max temperatures.
 *
 * Smoothly transitions from blue (coldest) through green (median) to red (warmest).
 *
 * @param temp Value to map.
 * @param minTemp Global baseline minimum temperature (maps to blue).
 * @param maxTemp Global baseline maximum temperature (maps to red).
 *
 * @return Color Structure containing RGB color channels (0-255).
 */
Color GetColor(double temp, double minTemp, double maxTemp);
