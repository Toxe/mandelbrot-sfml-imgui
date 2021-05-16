#pragma once

#include "clock/clock.h"
#include "mandelbrot/mandelbrot.h"
#include "window/window.h"

class CommandLine;

class App {
    Window window_;

    Clock frame_time_clock_;
    Duration elapsed_time_;

public:
    App(const CommandLine& cli);

    [[nodiscard]] Window& window() { return window_; };
    [[nodiscard]] Duration elapsed_time() const { return elapsed_time_; };
    [[nodiscard]] bool running() const { return window_.is_open(); };

    void next_frame();
};
