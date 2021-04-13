#include <algorithm>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "gradient.h"
#include "supervisor.h"

const FractalSection default_fractal_section = {-0.8, 0.0, 2.0};
const int default_max_iterations = 5000;
const int default_area_size = 100;

extern std::mutex paint_mtx;
extern std::atomic<Phase> supervisor_phase;

bool is_rendering = false;
float render_time = 0.0f;
sf::Clock render_clock;

void poll_events(sf::RenderWindow& window)
{
    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            supervisor_stop();
            window.close();
        } else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
            supervisor_stop();
            window.close();
        }
    }
}

void render_ui(sf::RenderWindow& window, sf::Clock& clock, sf::Image& image, SupervisorImageRequest& image_request)
{
    static std::vector<float> fps(120);
    static std::size_t values_offset = 0;

    const Phase phase = supervisor_phase;
    const ImVec4 gray_text{0.6f, 0.6f, 0.6f, 1.0f};
    const auto elapsed_time = clock.restart();

    ImGui::SFML::Update(window, elapsed_time);
    ImGui::Begin("Mandelbrot", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    const float elapsed_time_as_econds = elapsed_time.asSeconds();
    const float current_fps = 1.0f / elapsed_time_as_econds;
    const auto fps_label = fmt::format("{:.1f} FPS ({:.3f} ms/frame)", current_fps, 1000.0f * elapsed_time_as_econds);
    fps[values_offset] = current_fps;
    values_offset = (values_offset + 1) % fps.size();
    ImGui::PlotLines("", fps.data(), static_cast<int>(fps.size()), static_cast<int>(values_offset), fps_label.c_str(), 0.0f, 1.5f * std::max(65.0f, *std::max_element(fps.begin(), fps.end())), ImVec2(0, 100.0f));

    ImGui::Text("image size: %dx%d", image.getSize().x, image.getSize().y);

    ImGui::Separator();

    if (is_rendering) {
        if (phase == Phase::Idle)
            is_rendering = false;

        render_time = render_clock.getElapsedTime().asSeconds();
    }

    ImGui::Text("status: %s", supervisor_phase_name(phase));
    ImGui::Text("render time: %.3fs", render_time);

    ImGui::Separator();

    ImGui::TextColored(gray_text, "Left/right click: zoom in/out");
    ImGui::TextColored(gray_text, "Left drag: zoom in area");
    ImGui::TextColored(gray_text, "Right drag: move around");
    ImGui::TextColored(gray_text, "Space: show/hide UI");
    ImGui::TextColored(gray_text, "   F1: fullscreen");
    ImGui::TextColored(gray_text, "  ESC: quit");

    ImGui::NewLine();

    ImGui::InputDouble("center_x", &image_request.fractal_section.center_x, 0.1, 1.0);
    ImGui::InputDouble("center_y", &image_request.fractal_section.center_y, 0.1, 1.0);
    ImGui::InputDouble("fractal height", &image_request.fractal_section.height, 0.1, 1.0);
    ImGui::InputInt("iterations", &image_request.max_iterations, 100, 1000);
    ImGui::InputInt("tile size", &image_request.area_size, 100, 500);

    if (phase == Phase::Idle) {
        if (ImGui::Button("Calculate")) {
            is_rendering = true;
            render_clock.restart();
            render_time = 0.0f;
            supervisor_calc_image(image_request);
        }
    }

    ImGui::SameLine();

    if (phase == Phase::Idle) {
        if (ImGui::Button("Reset")) {
            image_request = SupervisorImageRequest{default_max_iterations, default_area_size, {static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, default_fractal_section};
            is_rendering = true;
            render_clock.restart();
            render_time = 0.0f;
            supervisor_calc_image(image_request);
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
    ImGuiIO& io = ImGui::GetIO();
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

    SupervisorImageRequest image_request{default_max_iterations, default_area_size, {static_cast<int>(image.getSize().x), static_cast<int>(image.getSize().y)}, default_fractal_section};

    auto supervisor = supervisor_start(image, texture, std::thread::hardware_concurrency(), gradient);

    sf::Clock clock;

    while (window.isOpen()) {
        poll_events(window);
        render_ui(window, clock, image, image_request);
        render_window(window, sprite);
    }

    ImGui::SFML::Shutdown();

    supervisor.wait();

    return 0;
}
