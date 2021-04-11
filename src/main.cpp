#include <algorithm>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>

#include "gradient.h"
#include "task_master.h"

const int max_iterations = 5000;

extern std::mutex paint_mtx;

void poll_events(sf::RenderWindow& window)
{
    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            task_master_stop();
            window.close();
        }
        else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
            task_master_stop();
            window.close();
        }
    }
}

void render_ui(sf::RenderWindow& window, sf::Clock& clock, sf::Image& image, ImageRequest& mandelbrot_params)
{
    ImGui::SFML::Update(window, clock.restart());
    ImGui::Begin("Mandelbrot");
    ImGui::Text("%.1f FPS (%.3f ms/frame)", 1.0 / ImGui::GetIO().DeltaTime, 1000.0 * ImGui::GetIO().DeltaTime);

    // if (ImGui::InputInt("max_iterations", &mandelbrot_params.max_iterations, 100, 1000))
    //     mandelbrot_params.max_iterations = std::clamp(mandelbrot_params.max_iterations, 10, 10000);

    if (ImGui::InputDouble("center_x", &mandelbrot_params.section.center_x, 0.1, 1.0))
        mandelbrot_params.section.center_x = std::clamp(mandelbrot_params.section.center_x, -5.0, 5.0);

    if (ImGui::InputDouble("center_y", &mandelbrot_params.section.center_y, 0.1, 1.0))
        mandelbrot_params.section.center_y = std::clamp(mandelbrot_params.section.center_y, -5.0, 5.0);

    if (ImGui::InputDouble("fractal_height", &mandelbrot_params.section.height, 0.1, 1.0))
        mandelbrot_params.section.height = std::clamp(mandelbrot_params.section.height, 0.0, 5.0);

    if (!task_master_working())
        if (ImGui::Button("Render"))
            task_master_image_request(mandelbrot_params);

    ImGui::SameLine();

    if (!task_master_working()) {
        if (ImGui::Button("Reset")) {
            mandelbrot_params = ImageRequest{{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, {-0.8, 0.0, 2.0}, max_iterations};
            task_master_image_request(mandelbrot_params);
        }
    }

    ImGui::End();
}

void render_window(sf::RenderWindow& window, sf::Sprite& sprite)
{
    window.clear();

    {
        std::lock_guard<std::mutex> lock(paint_mtx);
        window.draw(sprite);
    }

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

    ImGui::SFML::Init(window, false);
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig font_cfg;
    io.Fonts->Clear();
    font_cfg.SizePixels = 25.0f;
    io.Fonts->AddFontDefault(&font_cfg);
    ImGui::SFML::UpdateFontTexture();

    sf::Image image;
    image.create(window_width, window_height);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    ImageRequest mandelbrot_params{{static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, {-0.8, 0.0, 2.0}, max_iterations};
    auto task_master = task_master_start(image, texture, std::thread::hardware_concurrency(), mandelbrot_params.max_iterations, gradient);

    sf::Clock clock;

    while (window.isOpen()) {
        poll_events(window);
        render_ui(window, clock, image, mandelbrot_params);
        render_window(window, sprite);
    }

    ImGui::SFML::Shutdown();

    task_master.wait();

    return 0;
}
