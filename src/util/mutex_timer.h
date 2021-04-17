#pragma once

#include <string>
#include <thread>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

class MutexTimer {
    int count_ = 0;
    std::chrono::nanoseconds sum_ = 0ns;
    std::string mtx_name_;
    std::string label_;

public:
    MutexTimer(const std::string& mutex_name, const std::string& label) : mtx_name_{mutex_name}, label_{label} { }

    void update(std::chrono::steady_clock::time_point t0)
    {
        const auto t = std::chrono::high_resolution_clock::now() - t0;
        ++count_;
        sum_ += t;
    }

    void summary()
    {
        if (count_ > 0)
            spdlog::debug("[{}] {}: {} calls, {}, {} per call", mtx_name_, label_, count_, format_duration(sum_), format_duration(sum_ / count_));
        else
            spdlog::debug("[{}] {}: no calls", mtx_name_, label_);
    }

    std::string format_duration(std::chrono::nanoseconds t)
    {
        if (t < 1us)
            return fmt::format("{}", t);
        else if (t >= 1us && t < 1ms)
            return fmt::format("{}", std::chrono::duration_cast<std::chrono::microseconds>(t));
        else
            return fmt::format("{}", std::chrono::duration_cast<std::chrono::milliseconds>(t));
    }
};
