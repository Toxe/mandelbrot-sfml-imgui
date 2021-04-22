#include "app.h"

#include <spdlog/spdlog.h>

#include "cli.h"
#include "ui.h"

App::App(const CLI& cli)
    : window_{Window(cli)}, supervisor_{Supervisor(window_)}
{
    gradient_ = load_gradient("assets/gradients/benchmark.gradient");
    supervisor_.run(cli.num_threads(), gradient_);
}

void App::next_frame()
{
    supervisor_phase_ = supervisor_.get_phase();
    elapsed_time_ = frame_time_clock_.restart();

    window_.next_frame(elapsed_time_);
}

void App::poll_events(UI& ui)
{
    sf::Event event;

    while (window_.poll_event(event)) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                if (supervisor_phase_ == Phase::Idle)
                    ui.calculate_image(*this);
            } else if (event.key.code == sf::Keyboard::Space) {
                ui.toggle_visibility();
            } else if (event.key.code == sf::Keyboard::F1) {
                ui.toggle_help();
            }
        }
    }
}

void App::render()
{
    window_.render();
}

void App::calculate_image(SupervisorImageRequest& image_request)
{
    image_request.image_size = window_.size();
    supervisor_.calculate_image(image_request);
}

void App::cancel_calculation()
{
    supervisor_.cancel_calculation();
}

void App::shutdown()
{
    supervisor_.shutdown();
}
