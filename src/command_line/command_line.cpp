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

    fullscreen_ = false;
    num_threads_ = static_cast<int>(std::thread::hardware_concurrency());
    font_size_ = 0;

    CLI::App app{description};
    app.add_flag("-v", log_level_flag, "log level (-v: verbose, -vv: debug)");
    app.add_flag("-f,--fullscreen", fullscreen_, fmt::format("fullscreen (default: {})", fullscreen_));
    app.add_option("-n,--threads", num_threads_, fmt::format("number of threads (default: number of concurrent threads supported by the system: {})", num_threads_));
    app.add_option("--font-size", font_size_, "UI font size in pixels");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& error) {
        show_usage_and_exit(app, nullptr, error);
    }

    default_window_video_mode_ = default_video_mode(false);
    default_fullscreen_video_mode_ = default_video_mode(true);
    video_mode_ = fullscreen_ ? default_fullscreen_video_mode_ : default_window_video_mode_;

    if (font_size_ == 0)
        font_size_ = default_font_size(video_mode_, fullscreen_);

    auto log_level = (log_level_flag >= 1) ? spdlog::level::info : spdlog::level::warn;

    if (log_level_flag >= 2)
        log_level = spdlog::level::debug;

    spdlog::set_level(log_level);
    spdlog::info("command line option --fullscreen: {}", fullscreen_);
    spdlog::info("command line option --threads: {}", num_threads_);
    spdlog::info("command line option --font-size: {}", font_size_);
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
        // init window at half desktop height and 4:3 aspect ratio
        const auto desktop = sf::VideoMode::getDesktopMode();
        const unsigned int height = desktop.height / 2;
        const unsigned int width = 4 * height / 3;
        return sf::VideoMode{width, height};
    }
}

int CommandLine::default_font_size(const sf::VideoMode& video_mode, const bool fullscreen) const
{
    return static_cast<int>(video_mode.height) / (fullscreen ? 96 : 48);
}
