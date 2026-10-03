#pragma once

#include <chrono>

/**
 * @file Timer.h
 *
 * @brief High-resolution benchmarking timer.
 *
 * Employs std::chrono::high_resolution_clock for precision profiling
 * across serial and multi-threaded algorithm stages.
 */
class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop_time;
    bool running = false;

public:
    /**
     * @brief Constructs the timer and begins measurement immediately.
     */
    Timer() { start(); }

    /**
     * @brief Starts or resets the timer to current timestamp.
     */
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    /**
     * @brief Stops the timer and records stop timestamp.
     */
    void stop() {
        stop_time = std::chrono::high_resolution_clock::now();
        running = false;
    }

    /**
     * @brief Computes elapsed duration in seconds.
     *
     * If the timer is active, measures up to the current instant.
     * If stopped, returns duration between start and stop points.
     *
     * @return Elapsed time in seconds as floating-point value.
     */
    [[nodiscard]] double elapsedSeconds() const {
        const auto end_time = running ? std::chrono::high_resolution_clock::now() : stop_time;
        const std::chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }

    /**
     * @brief Computes elapsed duration in milliseconds.
     *
     * @return Elapsed time in milliseconds as floating-point value.
     */
    [[nodiscard]] double elapsedMilliseconds() const { return elapsedSeconds() * 1000.0; }
};
