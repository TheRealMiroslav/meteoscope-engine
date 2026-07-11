<div align="center">

# 🌦️ UPP Detektor Meteorologických Anomálií

[![C++](https://img.shields.io/badge/C++-20-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.8+-green.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg?style=for-the-badge)]()
[![Version](https://img.shields.io/badge/Version-1.0.0-lightgrey.svg?style=for-the-badge)]()

*Vysoce výkonný nástroj pro zpracování a vizualizaci meteorologických dat.*

</div>

---

## 📑 Obsah

- [O projektu](#-o-projektu)
- [Klíčové vlastnosti](#-klíčové-vlastnosti)
- [Technologie](#%EF%B8%8F-technologie)
- [Začínáme](#-začínáme)
  - [Prerekvizity](#prerekvizity)
  - [Instalace](#instalace)
- [Použití](#-použití)
- [Příspěvky](#-příspěvky)
- [Licence](#-licence)

---

## 📖 O projektu

**UPP Detektor Meteorologických Anomálií** je vysoce výkonná C++ aplikace navržená ke zpracování velkých objemů meteorologických dat, detekci anomálií v počasí a generování vizuálních teplotních map (SVG) spolu se strukturovanými CSV reporty. Aplikace byla vyvinuta pro efektivní zpracování rozsáhlých datových sad a využívá pokročilé metody souběžného programování. Nabízí sériový i paralelní režim spuštění, což výrazně zkracuje dobu zpracování masivních dávek dat.

Ať už analyzujete historické teplotní výkyvy napříč Českou republikou, nebo validujete klimatické modely, tento nástroj poskytuje rychlost a přesnost vyžadovanou pro moderní datové pipelines.

---

## ✨ Klíčové vlastnosti

- **🚀 Dva režimy zpracování:** Přepínejte mezi `--serial` (sériovým) a `--parallel` (paralelním) režimem pro optimalizaci výkonu na základě vašeho hardwaru.
- **🔍 Pokročilá detekce anomálií:** Přesně identifikuje extrémní výkyvy počasí a teplotní změny ze surových dat z měření.
- **🗺️ Generování SVG teplotních map:** Automaticky mapuje meteorologické anomálie do SVG map (např. `czmap.svg`) pomocí dynamického mapování barev.
- **📊 Export a parsování CSV:** Robustní načítání dat o stanicích a měřeních se strukturovaným, snadno analyzovatelným CSV výstupem.
- **⏱️ Precizní profilování:** Vestavěné časovače poskytují detailní telemetrii fází načítání a zpracování dat.
- **🌐 Podpora UTF-8:** Plně nakonfigurováno pro bezproblémové zpracování lokalizovaných názvů stanic a diakritiky.

---

## 🛠️ Technologie

| Technologie | Popis |
| :--- | :--- |
| ![C++20](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) | Jádro aplikační logiky a souběžné zpracování. |
| ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white) | Sestavovací (build) systém a konfigurace projektu. |
| ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white) | Cílový operační systém. |

---

## 🚀 Začínáme

Postupujte podle těchto pokynů k získání, spuštění a testování kopie projektu na vašem lokálním stroji.

### Prerekvizity

Před pokračováním se ujistěte, že máte nainstalováno následující:

*   **C++ Kompilátor:** Kompilátor plně podporující standard C++20 (např. MSVC, GCC 10+, Clang 11+).
*   **CMake:** Verze 3.8 nebo vyšší.
*   **Git:** Pro naklonování repozitáře.

### Instalace

1.  **Klonování repozitáře**
    ```bash
    git clone https://github.com/your-username/upp-meteorological-detector.git
    cd upp-meteorological-detector
    ```

2.  **Vytvoření build adresáře**
    ```bash
    mkdir build
    cd build
    ```

3.  **Generování build souborů pomocí CMake**
    ```bash
    cmake ..
    ```

4.  **Sestavení aplikace**
    ```bash
    cmake --build . --config Release
    ```

---

## 💻 Použití

Spustitelný soubor vyžaduje přesně tři argumenty: CSV se stanicemi, CSV s měřeními a přepínač režimu zpracování.

### Sériové zpracování (Jedno vlákno)
Ideální pro menší datové sady nebo ladění (debugging).
```bash
./upp_sp1 ../vstupniData/stanice.csv ../vstupniData/mereni.csv --serial
```

### Paralelní zpracování (Vícevláknové)
Doporučeno pro maximální výkon u velkých datových sad (`_big.csv`, `_med.csv`).
```bash
./upp_sp1 ../vstupniData/stanice_big.csv ../vstupniData/mereni_big.csv --parallel
```

### Očekávaný výstup
Po úspěšném dokončení program vypíše telemetrická data do konzole:
```text
================================================
       METEOROLOGICKA ANALYZA DOKONCENA         
================================================
 Rezim:            PARALELNI
 Pocet stanic:     150
 Pocet mereni:     1500000
------------------------------------------------
 Cas nacitani:     0.450 s
 Cas zpracovani:   1.200 s
------------------------------------------------
 CELKOVY CAS:      1.650 s
================================================
```
Vygenerované SVG mapy a CSV reporty budou uloženy do nakonfigurovaných výstupních adresářů (např. `maps/serial_maps/` nebo `maps/parallel_maps/`).

---

## 🤝 Příspěvky

Příspěvky komunity dělají z open-source tak úžasné místo k učení, inspiraci a tvorbě. Jakékoli příspěvky, které vytvoříte, jsou **velmi vítány**.

1. Forkněte si projekt (Fork)
2. Vytvořte svou Feature větev (`git checkout -b feature/SkvelaNovaFunkce`)
3. Zacommitujte své změny (`git commit -m 'Pridana SkvelaNovaFunkce'`)
4. Pushněte do větve (`git push origin feature/SkvelaNovaFunkce`)
5. Otevřete Pull Request

---

## 📄 Licence

Distribuováno pod licencí MIT. Pro více informací si prohlédněte soubor `LICENSE`.

<div align="center">
  <sub>Vytvořeno s ❤️ · 2026</sub>
</div>
