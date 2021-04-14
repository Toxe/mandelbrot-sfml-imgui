#include "app.h"

#include <spdlog/spdlog.h>
#include <imgui-SFML.h>

#include "supervisor.h"

extern std::mutex paint_mtx;

App::App()
{
    // init window at half desktop height and 4:3 aspect ratio
    const auto desktop = sf::VideoMode::getDesktopMode();
    const auto window_height = desktop.height / 2;
    const auto window_width = 4 * window_height / 3;

    spdlog::info("init window {}x{}", window_width, window_height);

    window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height), "Mandelbrot");
    window_->setVerticalSyncEnabled(true);
    window_->requestFocus();
}

void App::poll_events()
{
    sf::Event event;

    while (window_->pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            supervisor_stop();
            window_->close();
        } else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
            supervisor_stop();
            window_->close();
        }
    }
}

void App::render(sf::Sprite& sprite)
{
    window_->clear();

    {
        std::lock_guard<std::mutex> lock(paint_mtx);
        window_->draw(sprite);
    }

    ImGui::SFML::Render(*window_);
    window_->display();
}
