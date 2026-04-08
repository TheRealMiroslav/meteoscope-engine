#pragma once

struct Color {
    int r, g, b;
};

Color GetColor(double temp, double minTemp, double maxTemp);