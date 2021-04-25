#include "window.h"

#include <spdlog/spdlog.h>
#include <imgui-SFML.h>
#include <imgui.h>

#include "cli.h"

Window::Window(const CLI& cli)
    : is_fullscreen_{cli.fullscreen()}, window_video_mode_{cli.default_window_video_mode()}, fullscreen_video_mode_{cli.default_fullscreen_video_mode()}
{
    // create window
    spdlog::info("init {} mode {}x{}", is_fullscreen_ ? "fullscreen" : "window", cli.video_mode().width, cli.video_mode().height);

    auto style = is_fullscreen_ ? sf::Style::Fullscreen : sf::Style::Default;
    window_ = std::make_unique<sf::RenderWindow>(cli.video_mode(), title_, style);
    window_->setVerticalSyncEnabled(true);
    window_->requestFocus();

    // create render texture and sprite that shows the image
    sf::Image image;
    image.create(cli.video_mode().width, cli.video_mode().height);

    texture_ = std::make_unique<sf::Texture>();
    texture_->loadFromImage(image);

    sprite_ = std::make_unique<sf::Sprite>();
    sprite_->setTexture(*texture_);

    // init ImGui & ImGui-SFML and load a custom font
    ImGui::SFML::Init(*window_, false);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("assets/fonts/Inconsolata-SemiBold.ttf", static_cast<float>(cli.font_size()));
    ImGui::SFML::UpdateFontTexture();
}

[[nodiscard]] bool Window::handle_internal_event(const sf::Event& event)
{
    if (event.type == sf::Event::Closed) {
        close();
        return true;
    } else if (event.type == sf::Event::Resized) {
        adjust_view_to_window_size();
        return true;
    } else if (event.type == sf::Event::KeyPressed) {
        if (ImGui::GetIO().WantCaptureKeyboard)
            return true;

        if (event.key.code == sf::Keyboard::Escape) {
            close();
            return true;
        } else if (event.key.code == sf::Keyboard::F10) {
            toggle_fullscreen();
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool Window::poll_event(sf::Event& event)
{
    while (window_->pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (handle_internal_event(event))
            continue;

        return true;
    }

    return false;
}

[[nodiscard]] ImageSize Window::size() const
{
    const auto window_size = window_->getSize();
    return {static_cast<int>(window_size.x), static_cast<int>(window_size.y)};
}

void Window::next_frame(const Duration elapsed_time)
{
    ImGui::SFML::Update(*window_, sf::microseconds(elapsed_time.as_microseconds()));
}

void Window::render()
{
    window_->clear();

    {
        std::lock_guard<std::mutex> lock(mtx_);
        window_->draw(*sprite_);
    }

    ImGui::SFML::Render(*window_);
    window_->display();
}

void Window::toggle_fullscreen()
{
    if (is_fullscreen_) {
        window_->create(window_video_mode_, title_, sf::Style::Default);
        window_->setVerticalSyncEnabled(true);
        is_fullscreen_ = false;
    } else {
        // remember the current window position
        const auto size = window_->getSize();
        window_video_mode_.width = size.x;
        window_video_mode_.height = size.y;

        window_->create(fullscreen_video_mode_, title_, sf::Style::Fullscreen);
        window_->setVerticalSyncEnabled(true);
        is_fullscreen_ = true;
    }

    adjust_view_to_window_size();
}

void Window::close()
{
    if (window_->isOpen())
        window_->close();

    ImGui::SFML::Shutdown();
}

void Window::adjust_view_to_window_size()
{
    const auto size = window_->getSize();
    sf::FloatRect visibleArea(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y));
    window_->setView(sf::View(visibleArea));
}

void Window::resize_texture(const sf::Image& image)
{
    std::lock_guard<std::mutex> lock(mtx_);

    texture_.reset(new sf::Texture());
    texture_->loadFromImage(image);

    sprite_.reset(new sf::Sprite());
    sprite_->setTexture(*texture_);
}

void Window::update_texture(const sf::Image& image)
{
    std::lock_guard<std::mutex> lock(mtx_);
    texture_->update(image);
}

void Window::update_texture(const sf::Uint8* pixels, const CalculationArea& area)
{
    std::lock_guard<std::mutex> lock(mtx_);
    texture_->update(pixels, static_cast<unsigned int>(area.width), static_cast<unsigned int>(area.height),
                             static_cast<unsigned int>(area.x), static_cast<unsigned int>(area.y));
}
