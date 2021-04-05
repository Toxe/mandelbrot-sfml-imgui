#include <filesystem>
#include <string>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>

int main()
{
    // init window at half desktop height and 4:3 aspect ratio
    const auto desktop = sf::VideoMode::getDesktopMode();
    const int height = desktop.height / 2;
    const int width = 4 * height / 3;

    spdlog::info("init window {}x{}", width, height);

    sf::RenderWindow window(sf::VideoMode(width, height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(window);

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        ImGui::SFML::Update(window, clock.restart());

        window.clear(sf::Color::Black);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
