#include "app.h"

#include <spdlog/spdlog.h>

#include "cli.h"
#include "supervisor.h"
#include "ui.h"

App::App(const CLI& cli)
    : window_{Window(cli)}
{
    gradient_ = load_gradient("assets/gradients/benchmark.gradient");
}

void App::next_frame(const Supervisor& supervisor)
{
    supervisor_phase_ = supervisor.get_phase();
    elapsed_time_ = frame_time_clock_.restart();
    window_.next_frame(elapsed_time_);
}

void App::poll_events(Supervisor& supervisor, UI& ui)
{
    sf::Event event;

    while (window_.poll_event(event)) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                if (supervisor_phase_ == Phase::Idle)
                    ui.calculate_image(supervisor, window_.size());
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
