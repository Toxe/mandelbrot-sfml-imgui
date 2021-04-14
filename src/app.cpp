#include "app.h"

#include <spdlog/spdlog.h>
#include <imgui-SFML.h>

#include "supervisor.h"

extern std::mutex paint_mtx;

App::App(const CLI& cli)
{
    spdlog::info("init {} mode {}x{}", cli.fullscreen() ? "fullscreen" : "window", cli.video_mode().width, cli.video_mode().height);

    auto style = cli.fullscreen() ? sf::Style::Fullscreen : sf::Style::Default;
    window_ = std::make_unique<sf::RenderWindow>(cli.video_mode(), "Mandelbrot", style);
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
