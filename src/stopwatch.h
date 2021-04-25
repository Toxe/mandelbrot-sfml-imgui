#pragma once

#include "clock.h"

class Stopwatch {
    bool running_ = false;
    Duration elapsed_time_;
    Clock clock_;

public:
    [[nodiscard]] bool is_running() const { return running_; }

    void start()
    {
        running_ = true;
        elapsed_time_ = Duration();
        clock_.restart();
    }

    void stop()
    {
        running_ = false;
        elapsed_time_ = clock_.elapsed_time();
    }

    [[nodiscard]] Duration time()
    {
        if (!running_)
            return elapsed_time_;

        return elapsed_time_ = clock_.elapsed_time();
    }
};
