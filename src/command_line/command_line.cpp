#include "command_line.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

CommandLine::CommandLine(int argc, char* argv[])
{
    const auto description = "A multi-threaded C++ Mandelbrot renderer using SFML + ImGui.";
    int log_level_flag = 0;

    default_window_video_mode_ = default_video_mode(false);
    default_fullscreen_video_mode_ = default_video_mode(true);

    fullscreen_ = false;
    num_threads_ = static_cast<int>(std::thread::hardware_concurrency());
    font_size_ = 0;
    window_width_ = default_window_video_mode_.width;
    window_height_ = default_window_video_mode_.height;

    CLI::App app{description};
    app.add_flag("-v", log_level_flag, "log level (-v: INFO, -vv: DEBUG, -vvv: TRACE)");
    app.add_option("-n,--threads", num_threads_, fmt::format("number of threads (default: number of concurrent threads supported by the system: {})", num_threads_));
    app.add_option("--font-size", font_size_, "UI font size in pixels");
    auto opt_fullscreen = app.add_flag("-f,--fullscreen", fullscreen_, fmt::format("fullscreen (default: {})", fullscreen_));
    auto opt_width = app.add_option("--width", window_width_, fmt::format("window width (windowed mode only, default: {})", window_width_));
    auto opt_height = app.add_option("--height", window_height_, fmt::format("window height (windowed mode only, default: {})", window_height_));

    opt_fullscreen->excludes(opt_width)->excludes(opt_height);
    opt_width->check(CLI::PositiveNumber)->needs(opt_height)->excludes(opt_fullscreen);
    opt_height->check(CLI::PositiveNumber)->needs(opt_width)->excludes(opt_fullscreen);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& error) {
        show_usage_and_exit(app, nullptr, error);
    }

    default_window_video_mode_.width = window_width_;
    default_window_video_mode_.height = window_height_;
    video_mode_ = fullscreen_ ? default_fullscreen_video_mode_ : default_window_video_mode_;

    if (font_size_ == 0)
        font_size_ = default_font_size(video_mode_, fullscreen_);

    spdlog::level::level_enum log_level;

    switch (log_level_flag) {
        case  1: log_level = spdlog::level::info;   break;
        case  2: log_level = spdlog::level::debug;  break;
        case  3: log_level = spdlog::level::trace;  break;
        default: log_level = spdlog::level::warn;
    }

    spdlog::set_level(log_level);
    spdlog::debug("command line option --fullscreen: {}", fullscreen_);
    spdlog::debug("command line option --threads: {}", num_threads_);
    spdlog::debug("command line option --font-size: {}", font_size_);
    spdlog::debug("command line option --width: {}", window_width_);
    spdlog::debug("command line option --height: {}", window_height_);
}

void CommandLine::show_usage_and_exit(const CLI::App& app, const char* error_message = nullptr, const std::optional<CLI::ParseError>& error = {})
{
    if (error_message)
        fmt::print("\n{}\n", error_message);

    std::exit(error ? app.exit(error.value()) : 0);
}

sf::VideoMode CommandLine::default_video_mode(const int fullscreen) const
{
    if (fullscreen) {
        return sf::VideoMode::getFullscreenModes().front();
    } else {
        // init window at 75% desktop height and 4:3 aspect ratio
        const auto desktop = sf::VideoMode::getDesktopMode();
        const unsigned int height = (desktop.height * 3) / 4;
        const unsigned int width = 4 * height / 3;
        return sf::VideoMode{width, height};
    }
}

int CommandLine::default_font_size(const sf::VideoMode& video_mode, const bool fullscreen) const
{
    return static_cast<int>(video_mode.height) / (fullscreen ? 96 : 72);
}
