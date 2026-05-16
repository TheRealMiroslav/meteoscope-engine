#include "CsvParser.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <sstream>
#include <charconv>
#include <iostream>
#include <system_error>
#include <thread>

// ==============================================================================
// Sériové zpracování
// ==============================================================================

/**
 * @brief Načte seznam meteorologických stanic ze zadaného CSV souboru (sériově).
 *
 * @param path Cesta k CSV souboru obsahujícímu data o stanicích.
 *
 * @return std::vector<Station> Vektor naparsovaných stanic. V případě selhání vrací prázdný vektor.
 */
std::vector<Station> loadStationsSerial(const std::string &path) {
    // Otevření souboru s příznakem ate (at end) pro rychlé zjištění velikosti
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Načtení celého souboru do paměti (alokováno naráz pro minimalizaci I/O úzkých hrdel)
    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    // Vyhledání a přeskočení hlavičky CSV souboru
    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};

    std::vector<Station> result;
    // Předběžná alokace paměti založená na heuristice (cca 50 bajtů na řádek),
    // zamezuje zbytečným realokacím vektoru při vkládání.
    result.reserve(size / 50);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    // Line-by-line manuální parsování pro maximální výkon
    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n') lineEnd++;

        if (lineEnd > p) {
            Station station{};
            const char *curr = p;

            // Extrakce ID stanice
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, station.id);
            curr = ptr1;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Přeskočení sloupce s názvem stanice (data nejsou vyžadována)
            while (curr < lineEnd && *curr != ';') ++curr;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Extrakce zeměpisné šířky (Latitude)
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, station.lat);
            curr = ptr2;
            if (curr < lineEnd && *curr == ';') ++curr;

            // Extrakce zeměpisné délky (Longitude)
            std::from_chars(curr, lineEnd, station.lon);

            result.push_back(station);
        }
        p = lineEnd + 1;
    }

    return result;
}

/**
 * @brief Načte naměřené hodnoty ze zadaného CSV souboru (sériově).
 *
 * @param path Cesta k CSV souboru obsahujícímu měření.
 *
 * @return std::vector<Measurement> Vektor naparsovaných měření. V případě selhání vrací prázdný vektor.
 */
std::vector<Measurement> loadMeasurementSerial(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};

    std::vector<Measurement> result;
    // Heuristická rezervace paměti (cca 30 znaků na záznam o měření)
    result.reserve(size / 30);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n') lineEnd++;

        if (lineEnd > p) {
            Measurement m{};
            const char *curr = p;

            // ID stanice
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, m.id);
            curr = ptr1 + 1; // Přímý posun za středník

            // Pořadové číslo / Ordinal
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, m.ordinal);
            curr = ptr2 + 1;

            // Rok
            auto [ptr3, ec3] = std::from_chars(curr, lineEnd, m.year);
            curr = ptr3 + 1;

            // Měsíc
            auto [ptr4, ec4] = std::from_chars(curr, lineEnd, m.month);
            curr = ptr4 + 1;

            // Den měření - pro cílovou strukturu pravděpodobně nepotřebný, přeskakujeme
            while (curr < lineEnd && *curr != ';') ++curr;
            ++curr;

            // Hodnota měření (zpracování desetinné čárky)
            // std::from_chars striktně vyžaduje tečku pro oddělení desetinných míst,
            // proto nahrazujeme znak ',' za '.' přímo do lokálního bufferu.
            char valBuf[32];
            size_t len = 0;
            while (curr < lineEnd && *curr != '\r' && len < 31) {
                valBuf[len++] = (*curr == ',') ? '.' : *curr;
                ++curr;
            }
            std::from_chars(valBuf, valBuf + len, m.value);

            result.push_back(m);
        }
        p = lineEnd + 1;
    }
    return result;
}

// ==============================================================================
// Paralelní zpracování
// ==============================================================================

/**
 * @brief Načte naměřené hodnoty ze zadaného CSV souboru s využitím více vláken.
 *
 * Funkce rozdělí soubor na logické bloky a zpracuje je paralelně pomocí dostupných
 * hardwarových vláken, což výrazně zrychluje parsování u rozsáhlých datových sad.
 *
 * @param path Cesta k CSV souboru obsahujícímu měření.
 *
 * @return std::vector<Measurement> Vektor naparsovaných měření. V případě selhání vrací prázdný vektor.
 */
std::vector<Measurement> loadMeasurementParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};
    size_t startPos = headerEnd + 1;

    // Rozdělení datového bufferu na rovnoměrné části (tzv. chunks) podle počtu dostupných vláken
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *> > chunks;
    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        // Bezpečnostní zarovnání: Konec bloku se posouvá na konec nejbližšího řádku (\n),
        // aby nedošlo k přeříznutí datového záznamu (řádku) napůl mezi dvěma vlákny.
        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n') {
                endPos++;
            }
            if (endPos < size) endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    std::vector<std::vector<Measurement> > localResults(chunks.size());
    std::vector<std::thread> threads;

    // Spuštění parsování v dedikovaných vláknech. Každé vlákno zapisuje výhradně
    // do svého vektoru v localResults, aby nevznikl data race.
    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            localVec.reserve((end - p) / 30);

            while (p < end) {
                const char *lineEnd = p;
                while (lineEnd < end && *lineEnd != '\n') lineEnd++;

                if (lineEnd > p) {
                    Measurement m{};
                    const char *curr = p;

                    auto [ptr1, ec1] = std::from_chars(curr, lineEnd, m.id);
                    curr = ptr1 + 1;

                    auto [ptr2, ec2] = std::from_chars(curr, lineEnd, m.ordinal);
                    curr = ptr2 + 1;

                    auto [ptr3, ec3] = std::from_chars(curr, lineEnd, m.year);
                    curr = ptr3 + 1;

                    auto [ptr4, ec4] = std::from_chars(curr, lineEnd, m.month);
                    curr = ptr4 + 1;

                    while (curr < lineEnd && *curr != ';') ++curr;
                    ++curr;

                    char valBuf[32];
                    size_t len = 0;
                    while (curr < lineEnd && *curr != '\r' && len < 31) {
                        valBuf[len++] = (*curr == ',') ? '.' : *curr;
                        ++curr;
                    }
                    std::from_chars(valBuf, valBuf + len, m.value);

                    localVec.push_back(m);
                }

                p = lineEnd + 1;
            }
        });
    }

    // Synchronizace všech vláken před finální fází
    for (auto &t: threads) t.join();

    // Redukční fáze: Kalkulace celkové velikosti pro přesnou alokaci
    size_t totalMeasurements = 0;
    for (const auto &res: localResults) {
        totalMeasurements += res.size();
    }

    std::vector<Measurement> result;
    result.reserve(totalMeasurements);

    // Přesun (move) hotových datových vektorů z jednotlivých vláken do společného návratového vektoru
    for (auto &res: localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}

/**
 * @brief Načte seznam meteorologických stanic ze zadaného CSV souboru s využitím více vláken.
 *
 * @param path Cesta k CSV souboru obsahujícímu data o stanicích.
 *
 * @return std::vector<Station> Vektor naparsovaných stanic. V případě selhání vrací prázdný vektor.
 */
std::vector<Station> loadStationsParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size)) return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos) return {};
    size_t startPos = headerEnd + 1;

    // Výpočet a segmentace hranic pro multi-threading
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *> > chunks;
    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        // Zarovnání pozice konce na konec existujícího řádku (prevence splitů)
        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n') endPos++;
            if (endPos < size) endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    std::vector<std::vector<Station> > localResults(chunks.size());
    std::vector<std::thread> threads;

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            localVec.reserve((end - p) / 50);

            while (p < end) {
                const char *lineEnd = p;
                while (lineEnd < end && *lineEnd != '\n') lineEnd++;

                if (lineEnd > p) {
                    Station s{};
                    const char *curr = p;

                    auto [ptr1, ec1] = std::from_chars(curr, lineEnd, s.id);
                    curr = ptr1;
                    if (curr < lineEnd && *curr == ';') curr++;

                    while (curr < lineEnd && *curr != ';') curr++;
                    if (curr < lineEnd && *curr == ';') curr++;

                    auto [ptr2, ec2] = std::from_chars(curr, lineEnd, s.lat);
                    curr = ptr2;
                    if (curr < lineEnd && *curr == ';') curr++;

                    std::from_chars(curr, lineEnd, s.lon);

                    localVec.push_back(s);
                }
                p = lineEnd + 1;
            }
        });
    }

    for (auto &t: threads) t.join();

    // Spojení vláken a redukce výsledků (move semantics pro efektivitu)
    size_t totalStations = 0;
    for (const auto &res: localResults) totalStations += res.size();

    std::vector<Station> result;
    result.reserve(totalStations);

    for (auto &res: localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}
