#include "ColorMapper.h"

Color GetColor(const double temp, const double minTemp, const double maxTemp) {
    Color color{};

    const double tempNorm = (temp - minTemp) / (maxTemp - minTemp);

    if (tempNorm <= 0.5) {
        const double t = tempNorm * 2;

        color.r = static_cast<int>(0 + (255 - 0) * t);
        color.g = static_cast<int>(0 + (255 - 0) * t);
        color.b = static_cast<int>(255 + (0 - 255) * t);
    }
    else {
        const double t = (tempNorm - 0.5) * 2;

        color.r = static_cast<int>(255 + (255 - 255) * t);
        color.g = static_cast<int>(255 + (0 - 255) * t);
        color.b = static_cast<int>(0 + (0 - 0) * t);
    }

    return color;
}