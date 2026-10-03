#include "ColorMapper.h"

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
Color GetColor(const double temp, const double minTemp, const double maxTemp) {
    Color color{};

    // Normalize temperature into range [0.0, 1.0]
    const double tempNorm = (temp - minTemp) / (maxTemp - minTemp);

    // Lower half of thermal spectrum (Blue -> Green transition)
    if (tempNorm <= 0.5) {
        // Rescale [0.0, 0.5] to [0.0, 1.0] for linear interpolation
        const double t = tempNorm * 2;

        color.r = static_cast<int>(0 + (255 - 0) * t);
        color.g = static_cast<int>(0 + (255 - 0) * t);
        color.b = static_cast<int>(255 + (0 - 255) * t);
    }
    // Upper half of thermal spectrum (Green -> Red transition)
    else {
        // Rescale [0.5, 1.0] to [0.0, 1.0] for linear interpolation
        const double t = (tempNorm - 0.5) * 2;

        color.r = static_cast<int>(255 + (255 - 255) * t);
        color.g = static_cast<int>(255 + (0 - 255) * t);
        color.b = static_cast<int>(0 + (0 - 0) * t);
    }

    return color;
}
