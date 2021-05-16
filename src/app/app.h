#pragma once

#include "clock/clock.h"
#include "gradient/gradient.h"
#include "mandelbrot/mandelbrot.h"
#include "supervisor/supervisor.h"
#include "window/window.h"

class CommandLine;

class App {
    Gradient gradient_;
    Window window_;
    Supervisor supervisor_;

    Clock frame_time_clock_;
    Duration elapsed_time_;

public:
    App(const CommandLine& cli);

    [[nodiscard]] Gradient& gradient() { return gradient_; }
    [[nodiscard]] Window& window() { return window_; };
    [[nodiscard]] Supervisor& supervisor() { return supervisor_; };
    [[nodiscard]] SupervisorStatus& supervisor_status() { return supervisor_.status(); };
    [[nodiscard]] Duration elapsed_time() const { return elapsed_time_; };
    [[nodiscard]] bool running() const { return window_.is_open(); };

    void next_frame();

    void shutdown();
};
