#pragma once

#include "clock/clock.h"
#include "gradient/gradient.h"
#include "mandelbrot/mandelbrot.h"
#include "supervisor/supervisor.h"
#include "ui/ui.h"
#include "window/window.h"

class CommandLine;

class App {
    Gradient gradient_;
    Window window_;
    Supervisor supervisor_;
    UI ui_;

    Clock frame_time_clock_;
    Duration elapsed_time_;

public:
    App(const CommandLine& cli);

    [[nodiscard]] Gradient& gradient() { return gradient_; }
    [[nodiscard]] Window& window() { return window_; };
    [[nodiscard]] SupervisorStatus& supervisor_status() { return supervisor_.status(); };
    [[nodiscard]] Duration elapsed_time() const { return elapsed_time_; };
    [[nodiscard]] bool running() const { return window_.is_open(); };

    void next_frame();
    void poll_events();

    void render();

    void shutdown();

    void calculate_image(SupervisorImageRequest image_request);
    void colorize(SupervisorColorize colorize);
    void cancel_calculation();
    void change_num_threads(const int num_threads);
};
