#include "app.h"

#include <spdlog/spdlog.h>

#include "command_line/command_line.h"

App::App(const CommandLine& cli)
    : window_{Window(cli)}, supervisor_{Supervisor(window_)}, ui_{UI(cli)}
{
    gradient_ = load_gradient("benchmark");
    supervisor_.run(cli.num_threads(), gradient_);
}

void App::next_frame()
{
    elapsed_time_ = frame_time_clock_.restart();
    spdlog::trace("elapsed time: {}s ({} FPS)", elapsed_time_.as_seconds(), elapsed_time_.fps());

    window_.next_frame(elapsed_time_);
}

void App::poll_events()
{
    sf::Event event;

    while (window_.poll_event(event)) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                if (supervisor_.status().phase() == Phase::Idle)
                    ui_.calculate_image(*this);
            } else if (event.key.code == sf::Keyboard::Space) {
                ui_.toggle_visibility();
            } else if (event.key.code == sf::Keyboard::F1) {
                ui_.toggle_help();
            }
        }
    }
}

void App::render()
{
    if (running()) {
        ui_.render(*this);
        window_.render();
    }
}

void App::shutdown()
{
    supervisor_.shutdown();
}

void App::calculate_image(SupervisorImageRequest image_request)
{
    image_request.image_size = window_.size();
    supervisor_.calculate_image(image_request);
}

void App::colorize(SupervisorColorize colorize)
{
    colorize.image_size = window_.size();
    supervisor_.colorize(colorize);
}

void App::cancel_calculation()
{
    supervisor_.cancel_calculation();
}

void App::change_num_threads(const int num_threads)
{
    supervisor_.restart(num_threads);
}
