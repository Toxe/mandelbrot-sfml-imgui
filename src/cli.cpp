#include "cli.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

CLI::CLI(int argc, char* argv[])
{
    const auto description = "A multi-threaded C++ Mandelbrot renderer using SFML + ImGui.";
    auto log_level = spdlog::level::warn;
    bool show_help = false;

    fullscreen_ = false;
    num_threads_ = static_cast<int>(std::thread::hardware_concurrency());
    font_size_ = 25;

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        (clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show info log messages (verbose)" |
         clipp::option("-vv", "--debug").set(log_level, spdlog::level::debug)
            % "show debug log messages (very verbose)"),
        clipp::option("-f", "--fullscreen").set(fullscreen_, true)
            % fmt::format("fullscreen (default: {})", fullscreen_),
        (clipp::option("-n", "--threads") & clipp::integer("num_threads", num_threads_))
            % fmt::format("number of threads (default: number of concurrent threads supported by the system: {})", num_threads_),
        (clipp::option("--font-size") & clipp::integer("font_size", font_size_))
            % fmt::format("UI font size (default: {})", font_size_)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], description);

    video_mode_ = default_video_mode(fullscreen_);

    spdlog::set_level(log_level);
    spdlog::info("command line option --fullscreen: {}", fullscreen_);
    spdlog::info("command line option --threads: {}", num_threads_);
    spdlog::info("command line option --font-size: {}", font_size_);

    if (show_help)
        show_usage_and_exit(cli, argv[0], description);
}

void CLI::show_usage_and_exit(const clipp::group& cli, const std::string_view& argv0, const std::string_view& description) const
{
    std::string progname{std::filesystem::path{argv0}.filename().string()};
    fmt::print("{}", clipp::make_man_page(cli, progname)
        .prepend_section("DESCRIPTION", fmt::format("    {}", description)));

    std::exit(0);
}

sf::VideoMode CLI::default_video_mode(const int fullscreen) const
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

