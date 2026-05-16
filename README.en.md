<div align="center">

# 🌦️ UPP Meteorological Anomaly Detector

[![C++](https://img.shields.io/badge/C++-20-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.8+-green.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg?style=for-the-badge)]()
[![Version](https://img.shields.io/badge/Version-1.0.0-lightgrey.svg?style=for-the-badge)]()

*High-performance meteorological data processing and visualization engine.*

</div>

---

## 📑 Table of Contents

- [About The Project](#-about-the-project)
- [Key Features](#-key-features)
- [Built With](#%EF%B8%8F-built-with)
- [Getting Started](#-getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#-usage)
- [Roadmap](#-roadmap)
- [Contributing](#-contributing)
- [License](#-license)

---

## 📖 About The Project

**UPP Meteorological Anomaly Detector** is a high-performance C++ application designed to process large volumes of meteorological data, detect weather anomalies, and generate visual heatmaps (SVG) alongside structured CSV reports. Developed to handle significant datasets efficiently, the application leverages advanced concurrent programming to offer both serial and parallel execution modes, significantly reducing processing times for massive data batches.

Whether you're analyzing historical temperature swings across the Czech Republic or validating climate models, this tool provides the speed and accuracy required for modern data pipelines.

---

## ✨ Key Features

- **🚀 Dual Processing Modes:** Toggle between `--serial` and `--parallel` execution to optimize performance based on your hardware.
- **🔍 Advanced Anomaly Detection:** Accurately identifies extreme weather events and temperature fluctuations from raw measurement data.
- **🗺️ SVG Heatmap Generation:** Automatically maps meteorological anomalies onto SVG maps (e.g., `czmap.svg`) using dynamic color mapping.
- **📊 CSV Data Parsing & Export:** Robust ingestion of station and measurement data with structured, analyzable CSV outputs.
- **⏱️ Precision Profiling:** Built-in timers provide detailed telemetry on data loading and processing phases.
- **🌐 UTF-8 Support:** Fully configured to handle localized station names and characters out of the box.

---

## 🛠️ Built With

| Technology | Description |
| :--- | :--- |
| ![C++20](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) | Core application logic and concurrent processing. |
| ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white) | Build system and project configuration. |
| ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white) | Target operating system. |

---

## 🚀 Getting Started

Follow these instructions to get a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Ensure you have the following installed before proceeding:

*   **C++ Compiler:** A compiler that fully supports the C++20 standard (e.g., MSVC, GCC 10+, Clang 11+).
*   **CMake:** Version 3.8 or higher.
*   **Git:** To clone the repository.

### Installation

1.  **Clone the repo**
    ```bash
    git clone https://github.com/your-username/upp-meteorological-detector.git
    cd upp-meteorological-detector
    ```

2.  **Create a build directory**
    ```bash
    mkdir build
    cd build
    ```

3.  **Generate build files with CMake**
    ```bash
    cmake ..
    ```

4.  **Build the application**
    ```bash
    cmake --build . --config Release
    ```

---

## 💻 Usage

The executable requires exactly three arguments: the stations CSV, the measurements CSV, and the execution mode flag.

<details>
<summary><strong>Click to view usage examples</strong></summary>

### Serial Execution (Single Thread)
Ideal for smaller datasets or debugging.
```bash
./upp_sp1 ../vstupniData/stanice.csv ../vstupniData/mereni.csv --serial
```

### Parallel Execution (Multi-Threaded)
Recommended for maximum performance on large datasets (`_big.csv`, `_med.csv`).
```bash
./upp_sp1 ../vstupniData/stanice_big.csv ../vstupniData/mereni_big.csv --parallel
```

### Expected Output
Upon successful completion, the program will output telemetry data to the console:
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
Generated SVG maps and CSV reports will be saved to the configured output directories (e.g., `maps/serial_maps/` or `maps/parallel_maps/`).

</details>

---

## 🤝 Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📄 License

Distributed under the MIT License. See `LICENSE` for more information.

<div align="center">
  <sub>Built with ❤️ by the open-source community.</sub>
</div>