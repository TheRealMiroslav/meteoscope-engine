#pragma once

#include <chrono>

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop_time;
    bool running = false;

public:
    Timer() {
        start();
    }

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    void stop() {
        stop_time = std::chrono::high_resolution_clock::now();
        running = false;
    }

    [[nodiscard]] double elapsedSeconds() const {
        const auto end_time = running ? std::chrono::high_resolution_clock::now() : stop_time;
        const std::chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }

    [[nodiscard]] double elapsedMilliseconds() const {
        return elapsedSeconds() * 1000.0;
    }
};
