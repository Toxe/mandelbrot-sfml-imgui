#pragma once

#include <memory>
#include <mutex>

#include <SFML/Graphics.hpp>

#include "clock/duration.h"
#include "messages/messages.h"

class CommandLine;

class Window {
    const char* title_ = "Mandelbrot";

    bool is_fullscreen_;
    sf::VideoMode window_video_mode_;
    sf::VideoMode fullscreen_video_mode_;

    std::unique_ptr<sf::RenderWindow> window_;
    std::unique_ptr<sf::Texture> texture_;
    std::unique_ptr<sf::Sprite> sprite_;

    std::mutex mtx_;

    void adjust_view_to_window_size();

    [[nodiscard]] bool handle_internal_event(const sf::Event& event);

public:
    Window(const CommandLine& cli);

    [[nodiscard]] sf::Texture& texture() const { return *texture_; };
    [[nodiscard]] sf::Sprite& sprite() const { return *sprite_; };
    [[nodiscard]] bool is_open() const { return window_->isOpen(); };

    [[nodiscard]] ImageSize size() const;

    [[nodiscard]] bool poll_event(sf::Event& event);

    void next_frame(const Duration elapsed_time);
    void render();

    void toggle_fullscreen();
    void close();

    void resize_texture(const sf::Image& image);
    void update_texture(const sf::Image& image);
    void update_texture(const sf::Uint8* pixels, const CalculationArea& area);
};
