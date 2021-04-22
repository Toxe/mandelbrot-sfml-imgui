#pragma once

#include "gradient.h"
#include "mandelbrot.h"
#include "phase.h"
#include "window.h"

class CLI;
class Supervisor;
class UI;

class App {
    Gradient gradient_;
    Window window_;

    sf::Clock frame_time_clock_;
    sf::Time elapsed_time_;

    Phase supervisor_phase_ = Phase::Starting;

public:
    App(const CLI& cli);

    [[nodiscard]] Gradient& gradient() { return gradient_; }
    [[nodiscard]] Window& window() { return window_; };
    [[nodiscard]] Phase supervisor_phase() const { return supervisor_phase_; };
    [[nodiscard]] sf::Time elapsed_time() const { return elapsed_time_; };

    void next_frame(const Supervisor& supervisor);
    void poll_events(Supervisor& supervisor, UI& ui);

    void render();
};
