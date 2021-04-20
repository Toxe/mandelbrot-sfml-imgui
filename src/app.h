#pragma once

#include <memory>
#include <mutex>

#include <SFML/Graphics.hpp>

#include "cli.h"
#include "mandelbrot.h"
#include "phase.h"
#include "supervisor.h"

class UI;

class App {
    const char* window_title_ = "Mandelbrot";

    bool is_fullscreen_;
    sf::VideoMode window_video_mode_;
    sf::VideoMode fullscreen_video_mode_;

    std::unique_ptr<sf::RenderWindow> window_;
    std::unique_ptr<sf::Texture> texture_;
    std::unique_ptr<sf::Sprite> sprite_;

    sf::Clock frame_time_clock_;
    sf::Time elapsed_time_;

    std::mutex mtx_;
    Phase supervisor_phase_ = Phase::Starting;

public:
    App(const CLI& cli);

    sf::RenderWindow& window() const { return *window_; };
    sf::Texture& texture() const { return *texture_; };
    sf::Sprite& sprite() const { return *sprite_; };

    Phase supervisor_phase() const { return supervisor_phase_; };

    void resize_texture(const sf::Image& image);
    void update_texture(const sf::Image& image);
    void update_texture(const sf::Uint8* pixels, const CalculationArea& area);

    void next_frame();
    void poll_events(UI& ui);
    void render();

    void quit();

    void toggle_fullscreen();
    void adjust_view_to_window_size();

    sf::Time elapsed_time() const { return elapsed_time_; };
};
