#pragma once

#include "gradient.h"
#include "mandelbrot.h"
#include "supervisor.h"
#include "ui.h"
#include "window.h"

class CLI;

class App {
    Gradient gradient_;
    Window window_;
    Supervisor supervisor_;
    UI ui_;

    sf::Clock frame_time_clock_;
    sf::Time elapsed_time_;

public:
    App(const CLI& cli);

    [[nodiscard]] Gradient& gradient() { return gradient_; }
    [[nodiscard]] Window& window() { return window_; };
    [[nodiscard]] SupervisorStatus& supervisor_status() { return supervisor_.status(); };
    [[nodiscard]] sf::Time elapsed_time() const { return elapsed_time_; };
    [[nodiscard]] bool running() const { return window_.is_open(); };

    void next_frame();
    void poll_events();

    void render();

    void shutdown();

    void calculate_image(const SupervisorImageRequest image_request);
    void cancel_calculation();
    void change_num_threads(const int num_threads);
};
