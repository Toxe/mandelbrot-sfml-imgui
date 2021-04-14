#include "ui.h"

#include <atomic>

#include <fmt/core.h>
#include <imgui-SFML.h>
#include <imgui.h>

#include "supervisor.h"

const int default_max_iterations = 5000;
const int default_area_size = 100;
const FractalSection default_fractal_section = {-0.8, 0.0, 2.0};

extern std::mutex paint_mtx;
extern std::atomic<Phase> supervisor_phase;

UI::UI(const App& app, const CLI& cli) : supervisor_image_request_{make_default_supervisor_image_request(app)}
{
    ImGui::SFML::Init(app.window(), false);

    ImFontConfig font_cfg;
    font_cfg.SizePixels = static_cast<float>(cli.font_size());

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontDefault(&font_cfg);

    ImGui::SFML::UpdateFontTexture();
}

SupervisorImageRequest UI::make_default_supervisor_image_request(const App& app)
{
    const auto window_size = app.window().getSize();
    return {default_max_iterations, default_area_size, {static_cast<int>(window_size.x), static_cast<int>(window_size.y)}, default_fractal_section};
}

void UI::shutdown()
{
    ImGui::SFML::Shutdown();
}

void UI::render(const App& app, sf::Image& image)
{
    static std::vector<float> fps(120);
    static std::size_t values_offset = 0;

    const Phase phase = supervisor_phase;
    const ImVec4 gray_text{0.6f, 0.6f, 0.6f, 1.0f};
    const auto elapsed_time = frame_time_clock_.restart();

    ImGui::SFML::Update(app.window(), elapsed_time);
    ImGui::Begin("Mandelbrot", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    const float elapsed_time_as_econds = elapsed_time.asSeconds();
    const float current_fps = 1.0f / elapsed_time_as_econds;
    const auto fps_label = fmt::format("{:.1f} FPS ({:.3f} ms/frame)", current_fps, 1000.0f * elapsed_time_as_econds);
    fps[values_offset] = current_fps;
    values_offset = (values_offset + 1) % fps.size();
    ImGui::PlotLines("", fps.data(), static_cast<int>(fps.size()), static_cast<int>(values_offset), fps_label.c_str(), 0.0f, 1.5f * std::max(65.0f, *std::max_element(fps.begin(), fps.end())), ImVec2(0, static_cast<float>(4 * app.cli().font_size())));

    ImGui::Text("image size: %dx%d", image.getSize().x, image.getSize().y);

    ImGui::Separator();

    if (render_stopwatch_.is_running()) {
        if (phase == Phase::Idle)
            render_stopwatch_.stop();
    }

    ImGui::Text("status: %s", supervisor_phase_name(phase));
    ImGui::Text("render time: %.3fs", render_stopwatch_.time());

    ImGui::Separator();

    ImGui::TextColored(gray_text, "Left/right click: zoom in/out");
    ImGui::TextColored(gray_text, "Left drag: zoom in area");
    ImGui::TextColored(gray_text, "Right drag: move around");
    ImGui::TextColored(gray_text, "Space: show/hide UI");
    ImGui::TextColored(gray_text, "   F1: fullscreen");
    ImGui::TextColored(gray_text, "  ESC: quit");

    ImGui::NewLine();

    ImGui::InputDouble("center_x", &supervisor_image_request_.fractal_section.center_x, 0.1, 1.0);
    ImGui::InputDouble("center_y", &supervisor_image_request_.fractal_section.center_y, 0.1, 1.0);
    ImGui::InputDouble("fractal height", &supervisor_image_request_.fractal_section.height, 0.1, 1.0);
    ImGui::InputInt("iterations", &supervisor_image_request_.max_iterations, 100, 1000);
    ImGui::InputInt("tile size", &supervisor_image_request_.area_size, 100, 500);

    if (phase == Phase::Idle) {
        if (ImGui::Button("Calculate")) {
            render_stopwatch_.start();
            supervisor_calc_image(supervisor_image_request_);
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            supervisor_image_request_ = make_default_supervisor_image_request(app);
            render_stopwatch_.start();
            supervisor_calc_image(supervisor_image_request_);
        }
    }

    if (render_stopwatch_.is_running())
        if (ImGui::Button("Cancel"))
            supervisor_cancel_render();

    ImGui::End();
}
