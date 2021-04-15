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

    sf::Image image;
    image.create(cli.video_mode().width, cli.video_mode().height);

    texture_ = std::make_unique<sf::Texture>();
    texture_->loadFromImage(image);

    sprite_ = std::make_unique<sf::Sprite>();
    sprite_->setTexture(*texture_);
}

void App::poll_events()
{
    sf::Event event;

    while (window_->pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            supervisor_stop();
            window_->close();
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            supervisor_stop();
            window_->close();
        }
    }
}

void App::render()
{
    window_->clear();

    {
        std::lock_guard<std::mutex> lock(paint_mtx);
        window_->draw(*sprite_);
    }

    ImGui::SFML::Render(*window_);
    window_->display();
}

void App::resize_texture(const sf::Image& image)
{
    texture_.reset(new sf::Texture());
    texture_->loadFromImage(image);

    sprite_.reset(new sf::Sprite());
    sprite_->setTexture(*texture_);
}

void App::update_texture(const sf::Image& image)
{
    texture_->update(image);
}

void App::update_texture(const sf::Uint8* pixels, const CalculationArea& area)
{
    texture_->update(pixels, static_cast<unsigned int>(area.width), static_cast<unsigned int>(area.height),
                             static_cast<unsigned int>(area.x), static_cast<unsigned int>(area.y));
}
