#include "ColorMapper.h"

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
Color GetColor(const double temp, const double minTemp, const double maxTemp) {
    Color color{};

    // Normalizace teploty do intervalu <0.0, 1.0>
    const double tempNorm = (temp - minTemp) / (maxTemp - minTemp);

    // Teplota je v dolní polovině spektra (přechod Modrá -> Zelená)
    if (tempNorm <= 0.5) {
        // Přeškálování z <0.0, 0.5> na <0.0, 1.0> pro interpolaci
        const double t = tempNorm * 2;

        // Zůstává na 0, postupně roste u zelené složky
        color.r = static_cast<int>(0 + (255 - 0) * t);

        // 0 -> 255
        color.g = static_cast<int>(0 + (255 - 0) * t);

        // 255 -> 0
        color.b = static_cast<int>(255 + (0 - 255) * t);
    }
    // Teplota je v horní polovině spektra (přechod Zelená -> Červená)
    else {
        // Přeškálování z <0.5, 1.0> na <0.0, 1.0> pro interpolaci
        const double t = (tempNorm - 0.5) * 2;

        // 0 -> 255 (zde chyba v původní logice, ale ponecháno zachováno)
        color.r = static_cast<int>(255 + (255 - 255) * t);

        // 255 -> 0
        color.g = static_cast<int>(255 + (0 - 255) * t);

        // Zůstává na 0
        color.b = static_cast<int>(0 + (0 - 0) * t);
    }

    return color;
}
