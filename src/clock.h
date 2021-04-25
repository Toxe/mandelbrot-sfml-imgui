#pragma once

#include <chrono>

#include "duration.h"

class Clock {
    std::chrono::steady_clock clock_;
    std::chrono::steady_clock::time_point start_;

public:
    Clock() : start_{clock_.now()} {}

    Duration elapsed_time() {
        auto now = clock_.now();
        return Duration{now - start_};
    }

    Duration restart()
    {
        auto now = clock_.now();
        auto dur = now - start_;
        start_ = now;
        return Duration{dur};
    }
};
