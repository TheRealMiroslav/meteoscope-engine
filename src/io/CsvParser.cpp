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
// Serial Ingestion
// ==============================================================================

/**
 * @brief Ingests meteorological station records from a CSV file (sequentially).
 *
 * @param path Filesystem path to the stations CSV.
 *
 * @return std::vector<Station> Vector of parsed stations. Returns empty vector on failure.
 */
std::vector<Station> loadStationsSerial(const std::string &path) {
    // Open file with ate (at end) flag to rapidly query file byte size
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read full file into memory buffer in a single syscall to eliminate I/O overhead
    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size))
        return {};

    // Discover header line and advance pointer past it
    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos)
        return {};

    std::vector<Station> result;
    // Pre-allocate vector capacity based on average ~50 bytes per station line
    result.reserve(size / 50);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    // Fast zero-copy line parsing with std::from_chars
    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n')
            lineEnd++;

        if (lineEnd > p) {
            Station station{};
            const char *curr = p;

            // Extract station ID
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, station.id);
            curr = ptr1;
            if (curr < lineEnd && *curr == ';')
                ++curr;

            // Skip station name column (not required for processing)
            while (curr < lineEnd && *curr != ';')
                ++curr;
            if (curr < lineEnd && *curr == ';')
                ++curr;

            // Extract latitude
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, station.lat);
            curr = ptr2;
            if (curr < lineEnd && *curr == ';')
                ++curr;

            // Extract longitude
            std::from_chars(curr, lineEnd, station.lon);

            result.push_back(station);
        }
        p = lineEnd + 1;
    }

    return result;
}

/**
 * @brief Ingests time-series measurements from a CSV file (sequentially).
 *
 * @param path Filesystem path to the measurements CSV.
 *
 * @return std::vector<Measurement> Vector of parsed measurements. Returns empty vector on failure.
 */
std::vector<Measurement> loadMeasurementSerial(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size))
        return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos)
        return {};

    std::vector<Measurement> result;
    // Pre-allocate capacity (~30 bytes per measurement line)
    result.reserve(size / 30);

    const char *p = buffer.data() + headerEnd + 1;
    const char *end = buffer.data() + size;

    while (p < end) {
        const char *lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n')
            lineEnd++;

        if (lineEnd > p) {
            Measurement m{};
            const char *curr = p;

            // Station ID
            auto [ptr1, ec1] = std::from_chars(curr, lineEnd, m.id);
            curr = ptr1 + 1;

            // Ordinal sequence index
            auto [ptr2, ec2] = std::from_chars(curr, lineEnd, m.ordinal);
            curr = ptr2 + 1;

            // Observation year
            auto [ptr3, ec3] = std::from_chars(curr, lineEnd, m.year);
            curr = ptr3 + 1;

            // Observation month
            auto [ptr4, ec4] = std::from_chars(curr, lineEnd, m.month);
            curr = ptr4 + 1;

            // Skip day column
            while (curr < lineEnd && *curr != ';')
                ++curr;
            ++curr;

            // Parse floating-point value, normalizing decimal comma to dot
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
// Parallel Ingestion
// ==============================================================================

/**
 * @brief Concurrently ingests time-series measurements from a CSV file across worker threads.
 *
 * Splits the memory-buffered file into newline-aligned chunks processed concurrently
 * using std::thread::hardware_concurrency(), accelerating ingestion of massive datasets.
 *
 * @param path Filesystem path to the measurements CSV.
 *
 * @return std::vector<Measurement> Vector of parsed measurements. Returns empty vector on failure.
 */
std::vector<Measurement> loadMeasurementParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size))
        return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos)
        return {};
    size_t startPos = headerEnd + 1;

    // Partition buffer into chunks aligned with hardware concurrency
    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *>> chunks;
    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        // Line-boundary alignment: advance endPos to the next newline to prevent splitting rows
        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n') {
                endPos++;
            }
            if (endPos < size)
                endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    std::vector<std::vector<Measurement>> localResults(chunks.size());
    std::vector<std::thread> threads;

    // Launch worker threads. Each thread populates its isolated localResults partition (lock-free)
    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            localVec.reserve((end - p) / 30);

            while (p < end) {
                const char *lineEnd = p;
                while (lineEnd < end && *lineEnd != '\n')
                    lineEnd++;

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

                    while (curr < lineEnd && *curr != ';')
                        ++curr;
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

    for (auto &t : threads)
        t.join();

    // Reduction phase: compute total count for exact single-allocation merge
    size_t totalMeasurements = 0;
    for (const auto &res : localResults) {
        totalMeasurements += res.size();
    }

    std::vector<Measurement> result;
    result.reserve(totalMeasurements);

    for (auto &res : localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}

/**
 * @brief Concurrently ingests meteorological station records from a CSV file across worker threads.
 *
 * @param path Filesystem path to the stations CSV.
 *
 * @return std::vector<Station> Vector of parsed stations. Returns empty vector on failure.
 */
std::vector<Station> loadStationsParallel(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(buffer.data(), size))
        return {};

    const size_t headerEnd = buffer.find('\n');
    if (headerEnd == std::string::npos)
        return {};
    size_t startPos = headerEnd + 1;

    const size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());
    std::vector<std::pair<const char *, const char *>> chunks;
    const size_t approxChunk = (size - startPos) / nThreads;

    for (size_t i = 0; i < nThreads; ++i) {
        size_t endPos = (i == nThreads - 1) ? size : startPos + approxChunk;

        if (endPos < size) {
            while (endPos < size && buffer[endPos] != '\n')
                endPos++;
            if (endPos < size)
                endPos++;
        }

        if (startPos < endPos) {
            chunks.emplace_back(buffer.data() + startPos, buffer.data() + endPos);
        }
        startPos = endPos;
    }

    std::vector<std::vector<Station>> localResults(chunks.size());
    std::vector<std::thread> threads;

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([i, &chunks, &localResults]() {
            const char *p = chunks[i].first;
            const char *end = chunks[i].second;
            auto &localVec = localResults[i];

            localVec.reserve((end - p) / 50);

            while (p < end) {
                const char *lineEnd = p;
                while (lineEnd < end && *lineEnd != '\n')
                    lineEnd++;

                if (lineEnd > p) {
                    Station s{};
                    const char *curr = p;

                    auto [ptr1, ec1] = std::from_chars(curr, lineEnd, s.id);
                    curr = ptr1;
                    if (curr < lineEnd && *curr == ';')
                        curr++;

                    while (curr < lineEnd && *curr != ';')
                        curr++;
                    if (curr < lineEnd && *curr == ';')
                        curr++;

                    auto [ptr2, ec2] = std::from_chars(curr, lineEnd, s.lat);
                    curr = ptr2;
                    if (curr < lineEnd && *curr == ';')
                        curr++;

                    std::from_chars(curr, lineEnd, s.lon);

                    localVec.push_back(s);
                }
                p = lineEnd + 1;
            }
        });
    }

    for (auto &t : threads)
        t.join();

    size_t totalStations = 0;
    for (const auto &res : localResults)
        totalStations += res.size();

    std::vector<Station> result;
    result.reserve(totalStations);

    for (auto &res : localResults) {
        result.insert(result.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    return result;
}
