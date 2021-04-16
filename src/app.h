#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include "cli.h"
#include "mandelbrot.h"

class App {
    const char* window_title_ = "Mandelbrot";
    bool is_fullscreen_;
    sf::VideoMode window_video_mode_;
    sf::VideoMode fullscreen_video_mode_;
    std::unique_ptr<sf::RenderWindow> window_;
    std::unique_ptr<sf::Texture> texture_;
    std::unique_ptr<sf::Sprite> sprite_;

public:
    App(const CLI& cli);

    sf::RenderWindow& window() const { return *window_; };
    sf::Texture& texture() const { return *texture_; };
    sf::Sprite& sprite() const { return *sprite_; };

    void resize_texture(const sf::Image& image);
    void update_texture(const sf::Image& image);
    void update_texture(const sf::Uint8* pixels, const CalculationArea& area);

    void poll_events();
    void render();

    void toggle_fullscreen();
    void adjust_view_to_window_size();
};
