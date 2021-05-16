#pragma once

#include "clock/clock.h"

class App {
    Clock frame_time_clock_;
    Duration elapsed_time_;

public:
    [[nodiscard]] Duration elapsed_time() const { return elapsed_time_; };

    void next_frame();
};
