#include "app.h"

#include <spdlog/spdlog.h>
#include <imgui-SFML.h>

#include "supervisor.h"
#include "ui.h"

App::App(const CLI& cli)
    : is_fullscreen_{cli.fullscreen()}, window_video_mode_{cli.default_window_video_mode()}, fullscreen_video_mode_{cli.default_fullscreen_video_mode()}
{
    spdlog::info("init {} mode {}x{}", is_fullscreen_ ? "fullscreen" : "window", cli.video_mode().width, cli.video_mode().height);

    auto style = is_fullscreen_ ? sf::Style::Fullscreen : sf::Style::Default;
    window_ = std::make_unique<sf::RenderWindow>(cli.video_mode(), window_title_, style);
    window_->setVerticalSyncEnabled(true);
    window_->requestFocus();

    sf::Image image;
    image.create(cli.video_mode().width, cli.video_mode().height);

    texture_ = std::make_unique<sf::Texture>();
    texture_->loadFromImage(image);

    sprite_ = std::make_unique<sf::Sprite>();
    sprite_->setTexture(*texture_);
}

void App::next_frame()
{
    elapsed_time_ = frame_time_clock_.restart();
}

void App::poll_events(UI& ui)
{
    sf::Event event;

    while (window_->pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            supervisor_stop();
            window_->close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                supervisor_stop();
                window_->close();
            } else if (event.key.code == sf::Keyboard::Enter) {
                toggle_fullscreen();
            } else if (event.key.code == sf::Keyboard::Space) {
                ui.toggle_visibility();
            }
        } else if (event.type == sf::Event::Resized) {
            adjust_view_to_window_size();
        }
    }
}

void App::render()
{
    window_->clear();

    {
        std::lock_guard<std::mutex> lock(mtx_);
        window_->draw(*sprite_);
    }

    ImGui::SFML::Render(*window_);
    window_->display();
}

void App::resize_texture(const sf::Image& image)
{
    std::lock_guard<std::mutex> lock(mtx_);

    texture_.reset(new sf::Texture());
    texture_->loadFromImage(image);

    sprite_.reset(new sf::Sprite());
    sprite_->setTexture(*texture_);
}

void App::update_texture(const sf::Image& image)
{
    std::lock_guard<std::mutex> lock(mtx_);
    texture_->update(image);
}

void App::update_texture(const sf::Uint8* pixels, const CalculationArea& area)
{
    std::lock_guard<std::mutex> lock(mtx_);
    texture_->update(pixels, static_cast<unsigned int>(area.width), static_cast<unsigned int>(area.height),
                             static_cast<unsigned int>(area.x), static_cast<unsigned int>(area.y));
}

void App::toggle_fullscreen()
{
    if (is_fullscreen_) {
        window_->create(window_video_mode_, window_title_, sf::Style::Default);
        window_->setVerticalSyncEnabled(true);
        is_fullscreen_ = false;
    } else {
        // remember the current windowed video mode
        const auto size = window_->getSize();
        window_video_mode_.width = size.x;
        window_video_mode_.height = size.y;

        window_->create(fullscreen_video_mode_, window_title_, sf::Style::Fullscreen);
        window_->setVerticalSyncEnabled(true);
        is_fullscreen_ = true;
    }

    adjust_view_to_window_size();
}

void App::adjust_view_to_window_size()
{
    const auto size = window_->getSize();
    sf::FloatRect visibleArea(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y));
    window_->setView(sf::View(visibleArea));
}
