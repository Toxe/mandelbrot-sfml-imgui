#pragma once

#include <string_view>

#include <clipp.h>
#include <SFML/Window/VideoMode.hpp>

class CLI {
    bool fullscreen_;
    int num_threads_;
    int font_size_;
    sf::VideoMode video_mode_;
    sf::VideoMode default_window_video_mode_;
    sf::VideoMode default_fullscreen_video_mode_;

    void show_usage_and_exit(const clipp::group& cli, const std::string_view& argv0, const std::string_view& description) const;
    [[nodiscard]] sf::VideoMode default_video_mode(const int fullscreen) const;
    [[nodiscard]] int default_font_size(const sf::VideoMode& video_mode, const bool fullscreen) const;

public:
    CLI(int argc, char* argv[]);

    [[nodiscard]] bool fullscreen() const { return fullscreen_; }
    [[nodiscard]] int num_threads() const { return num_threads_; }
    [[nodiscard]] int font_size() const { return font_size_; }
    [[nodiscard]] sf::VideoMode video_mode() const { return video_mode_; };
    [[nodiscard]] sf::VideoMode default_window_video_mode() const { return default_window_video_mode_; };
    [[nodiscard]] sf::VideoMode default_fullscreen_video_mode() const { return default_fullscreen_video_mode_; };
};
