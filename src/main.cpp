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
    int width;
    int height;
    int max_iterations;
    double center_x;
    double center_y;
    double fractal_height;
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

void generate_mandelbrot(sf::Image& image, sf::Sprite& sprite, sf::Texture& texture, const Gradient& gradient, const int window_width, const int window_height, const MandelbrotParams& mandelbrot_params)
{
    std::vector<int> iterations_histogram(static_cast<std::size_t>(mandelbrot_params.max_iterations + 1));
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(mandelbrot_params.width * mandelbrot_params.height));

    mandelbrot_calc(mandelbrot_params.width, mandelbrot_params.height, mandelbrot_params.max_iterations, mandelbrot_params.center_x, mandelbrot_params.center_y, mandelbrot_params.fractal_height, iterations_histogram, results_per_point);
    mandelbrot_colorize(mandelbrot_params.max_iterations, gradient, image, iterations_histogram, results_per_point);

    texture.update(image);
    sprite.setScale(static_cast<float>(window_width) / static_cast<float>(mandelbrot_params.width), static_cast<float>(window_height) / static_cast<float>(mandelbrot_params.height));
}

void render_ui(sf::RenderWindow& window, sf::Clock& clock, sf::Image& image, sf::Sprite& sprite, sf::Texture& texture, const Gradient& gradient, const int window_width, const int window_height, MandelbrotParams& mandelbrot_params)
{
    ImGui::SFML::Update(window, clock.restart());
    ImGui::Begin("Mandelbrot");
    ImGui::Text("%.1f FPS (%.3f ms/frame)", 1.0 / ImGui::GetIO().DeltaTime, 1000.0 * ImGui::GetIO().DeltaTime);

    if (ImGui::InputInt("max_iterations", &mandelbrot_params.max_iterations, 100, 1000))
        mandelbrot_params.max_iterations = std::clamp(mandelbrot_params.max_iterations, 10, 10000);

    if (ImGui::InputDouble("center_x", &mandelbrot_params.center_x, 0.1, 1.0))
        mandelbrot_params.center_x = std::clamp(mandelbrot_params.center_x, -5.0, 5.0);

    if (ImGui::InputDouble("center_y", &mandelbrot_params.center_y, 0.1, 1.0))
        mandelbrot_params.center_y = std::clamp(mandelbrot_params.center_y, -5.0, 5.0);

    if (ImGui::InputDouble("fractal_height", &mandelbrot_params.fractal_height, 0.1, 1.0))
        mandelbrot_params.fractal_height = std::clamp(mandelbrot_params.fractal_height, 0.0, 5.0);

    if (ImGui::Button("Render"))
        generate_mandelbrot(image, sprite, texture, gradient, window_width, window_height, mandelbrot_params);

    ImGui::SameLine();

    if (ImGui::Button("Reset")) {
        mandelbrot_params = MandelbrotParams{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y), 100, -0.8, 0.0, 2.0};
        generate_mandelbrot(image, sprite, texture, gradient, window_width, window_height, mandelbrot_params);
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
    const int window_height = desktop.height / 2;
    const int window_width = 4 * window_height / 3;

    spdlog::info("init window {}x{}", window_width, window_height);

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(window);

    sf::Image image;
    image.create(100, 75);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    MandelbrotParams mandelbrot_params{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y), 100, -0.8, 0.0, 2.0};

    sf::Clock clock;

    while (window.isOpen()) {
        poll_events(window);
        render_ui(window, clock, image, sprite, texture, gradient, window_width, window_height, mandelbrot_params);
        render_window(window, sprite);
    }

    ImGui::SFML::Shutdown();

    return 0;
}
