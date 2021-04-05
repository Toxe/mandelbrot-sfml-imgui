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
#include "pixel_color.h"

void copy_mandelbrot_to_image(const std::vector<PixelColor>& image_data, sf::Image& image, const int image_width, const int image_height)
{
    for (int y = 0; y < image_height; ++y) {
        for (int x = 0; x < image_width; ++x) {
            const PixelColor& pixel = image_data[y * image_width + x];
            image.setPixel(x, y, {pixel.r, pixel.g, pixel.b});
        }
    }
}

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

void update_ui(sf::RenderWindow& window, sf::Clock& clock)
{
    ImGui::SFML::Update(window, clock.restart());
    ImGui::Begin("Mandelbrot");
    ImGui::Text("%.1f FPS (%.3f ms/frame)", 1.0 / ImGui::GetIO().DeltaTime, 1000.0 * ImGui::GetIO().DeltaTime);
    ImGui::End();
}

void generate_mandelbrot(sf::Image& image, sf::Sprite& sprite, sf::Texture& texture, const Gradient& gradient, const int window_width, const int window_height)
{
    const int image_width = image.getSize().x;
    const int image_height = image.getSize().y;
    const int max_iterations = 100;
    const double center_x = -0.8;
    const double center_y = 0.0;
    const double fractal_height = 2.0;

    std::vector<int> iterations_histogram(static_cast<std::size_t>(max_iterations + 1));
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(image_width * image_height));
    std::vector<PixelColor> image_data(static_cast<std::size_t>(image_width * image_height));

    mandelbrot_calc(image_width, image_height, max_iterations, center_x, center_y, fractal_height, iterations_histogram, results_per_point);
    mandelbrot_colorize(max_iterations, gradient, image_data, iterations_histogram, results_per_point);

    copy_mandelbrot_to_image(image_data, image, image_width, image_height);

    texture.update(image);
    sprite.setScale(static_cast<float>(window_width) / static_cast<float>(image_width), static_cast<float>(window_height) / static_cast<float>(image_height));
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

    sf::Clock clock;

    while (window.isOpen()) {
        poll_events(window);
        update_ui(window, clock);

        generate_mandelbrot(image, sprite, texture, gradient, window_width, window_height);

        render_window(window, sprite);
    }

    ImGui::SFML::Shutdown();

    return 0;
}
