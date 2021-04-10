#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>

#include "gradient.h"
#include "mandelbrot.h"

struct MandelbrotParams {
    ImageSize image_size;
    Section section;
    int max_iterations;
};

void poll_events(sf::RenderWindow& window)
{
    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape)
            window.close();
    }
}

void generate_mandelbrot(sf::Image& image, sf::Sprite& sprite, sf::Texture& texture, const Gradient& gradient, const sf::Vector2u window_size, const MandelbrotParams& mandelbrot_params)
{
    std::vector<int> iterations_histogram(static_cast<std::size_t>(mandelbrot_params.max_iterations + 1));
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(mandelbrot_params.image_size.width * mandelbrot_params.image_size.height));

    mandelbrot_calc(mandelbrot_params.image_size, mandelbrot_params.section, mandelbrot_params.max_iterations, iterations_histogram, results_per_point);
    mandelbrot_colorize(mandelbrot_params.max_iterations, gradient, image, iterations_histogram, results_per_point);

    texture.update(image);
    sprite.setScale(static_cast<float>(window_size.x) / static_cast<float>(mandelbrot_params.image_size.width), static_cast<float>(window_size.y) / static_cast<float>(mandelbrot_params.image_size.height));
}

void render_ui(sf::RenderWindow& window, sf::Clock& clock, sf::Image& image, sf::Sprite& sprite, sf::Texture& texture, const Gradient& gradient, const sf::Vector2u window_size, MandelbrotParams& mandelbrot_params)
{
    ImGui::SFML::Update(window, clock.restart());
    ImGui::Begin("Mandelbrot");
    ImGui::Text("%.1f FPS (%.3f ms/frame)", 1.0 / ImGui::GetIO().DeltaTime, 1000.0 * ImGui::GetIO().DeltaTime);

    if (ImGui::InputInt("max_iterations", &mandelbrot_params.max_iterations, 100, 1000))
        mandelbrot_params.max_iterations = std::clamp(mandelbrot_params.max_iterations, 10, 10000);

    if (ImGui::InputDouble("center_x", &mandelbrot_params.section.center_x, 0.1, 1.0))
        mandelbrot_params.section.center_x = std::clamp(mandelbrot_params.section.center_x, -5.0, 5.0);

    if (ImGui::InputDouble("center_y", &mandelbrot_params.section.center_y, 0.1, 1.0))
        mandelbrot_params.section.center_y = std::clamp(mandelbrot_params.section.center_y, -5.0, 5.0);

    if (ImGui::InputDouble("fractal_height", &mandelbrot_params.section.height, 0.1, 1.0))
        mandelbrot_params.section.height = std::clamp(mandelbrot_params.section.height, 0.0, 5.0);

    if (ImGui::Button("Render"))
        generate_mandelbrot(image, sprite, texture, gradient, window_size, mandelbrot_params);

    ImGui::SameLine();

    if (ImGui::Button("Reset")) {
        mandelbrot_params = MandelbrotParams{{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, {-0.8, 0.0, 2.0}, 100};
        generate_mandelbrot(image, sprite, texture, gradient, window_size, mandelbrot_params);
    }

    ImGui::End();
}

void render_window(sf::RenderWindow& window, sf::Sprite& sprite)
{
    window.clear();
    window.draw(sprite);
    ImGui::SFML::Render(window);
    window.display();
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    const auto gradient = load_gradient("assets/gradients/benchmark.gradient");

    // init window at half desktop height and 4:3 aspect ratio
    const auto desktop = sf::VideoMode::getDesktopMode();
    const auto window_height = desktop.height / 2;
    const auto window_width = 4 * window_height / 3;

    spdlog::info("init window {}x{}", window_width, window_height);

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(window);

    sf::Image image;
    image.create(100, 75);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    MandelbrotParams mandelbrot_params{{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, {-0.8, 0.0, 2.0}, 100};

    sf::Clock clock;

    while (window.isOpen()) {
        poll_events(window);
        render_ui(window, clock, image, sprite, texture, gradient, window.getSize(), mandelbrot_params);
        render_window(window, sprite);
    }

    ImGui::SFML::Shutdown();

    return 0;
}
