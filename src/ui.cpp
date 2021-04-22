#include "ui.h"

#include <algorithm>
#include <atomic>
#include <limits>

#include <fmt/core.h>
#include <imgui.h>

#include "app.h"
#include "cli.h"
#include "phase.h"
#include "supervisor.h"

const int default_max_iterations = 5000;
const int default_area_size = 100;
const FractalSection default_fractal_section = {-0.8, 0.0, 2.0};

UI::UI(const CLI& cli)
    : num_threads_{cli.num_threads()}, font_size_{static_cast<float>(cli.font_size())},
      supervisor_image_request_{make_default_supervisor_image_request()}
{
}

SupervisorImageRequest UI::make_default_supervisor_image_request()
{
    return {default_max_iterations, default_area_size, {0, 0}, default_fractal_section};
}

void UI::render(App& app)
{
    render_main_window(app);
    render_help_window();
}

void UI::render_main_window(App& app)
{
    static std::vector<float> fps(120);
    static std::size_t values_offset = 0;

    const Phase phase = app.supervisor_phase();
    const ImVec4 gray_text{0.6f, 0.6f, 0.6f, 1.0f};

    if (render_stopwatch_.is_running())
        if (phase == Phase::Idle)
            render_stopwatch_.stop();

    const float elapsed_time_in_seconds = app.elapsed_time().asSeconds();
    const float current_fps = 1.0f / elapsed_time_in_seconds;
    const auto fps_label = fmt::format("{:.1f} FPS ({:.3f} ms/frame)", current_fps, 1000.0f * elapsed_time_in_seconds);
    fps[values_offset] = current_fps;
    values_offset = (values_offset + 1) % fps.size();

    if (!is_visible_)
        return;

    const auto window_size = app.window().size();

    ImGui::Begin("Mandelbrot", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::PlotLines("", fps.data(), static_cast<int>(fps.size()), static_cast<int>(values_offset), fps_label.c_str(), 0.0f, 1.5f * std::max(65.0f, *std::max_element(fps.begin(), fps.end())), ImVec2(0, 4.0f * font_size_));

    ImGui::Text("image size: %dx%d", window_size.width, window_size.height);
    ImGui::Text("status: %s", phase_name(phase));
    ImGui::Text("render time: %.3fs", render_stopwatch_.time());

    if (ImGui::Button("Help (F1)"))
        toggle_help();

    ImGui::SameLine();

    if (ImGui::Button("Fullscreen (F10)"))
        app.window().toggle_fullscreen();

    ImGui::NewLine();

    input_int("number of threads", num_threads_, &changed_num_threads_, 1, 10, 1, 1'000);

    if (changed_num_threads_ && phase == Phase::Idle) {
        if (ImGui::Button("change")) {
            app.change_num_threads(num_threads_);
            changed_num_threads_ = false;
        }
    }

    ImGui::NewLine();

    input_double("center_x", supervisor_image_request_.fractal_section.center_x, nullptr, 0.1, 1.0, -5.0, 5.0);
    input_double("center_y", supervisor_image_request_.fractal_section.center_y, nullptr, 0.1, 1.0, -5.0, 5.0);
    input_double("fractal height", supervisor_image_request_.fractal_section.height, nullptr, 0.1, 1.0, 1000.0 * std::numeric_limits<double>::min(), 10.0);
    input_int("iterations", supervisor_image_request_.max_iterations, nullptr, 100, 1000, 10, 1'000'000);
    input_int("tile size", supervisor_image_request_.area_size, nullptr, 100, 500, 10, 10'000);

    if (phase == Phase::Idle) {
        if (ImGui::Button("Calculate"))
            calculate_image(app);

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            supervisor_image_request_ = make_default_supervisor_image_request();
            calculate_image(app);
        }
    }

    if (render_stopwatch_.is_running())
        if (ImGui::Button("Cancel"))
            app.cancel_calculation();

    ImGui::End();
}

void UI::render_help_window()
{
    if (show_help_) {
        ImGui::Begin("Help", &show_help_, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Left/right click: zoom in/out");
        ImGui::Text("Left drag: zoom in area");
        ImGui::Text("Right drag: move around");
        ImGui::Separator();
        ImGui::Text("Enter: calculate image");
        ImGui::Text("Space: show/hide UI");
        ImGui::Text("   F1: show/hide help");
        ImGui::Text("  F10: fullscreen");
        ImGui::Text("  ESC: quit");

        if (ImGui::Button("Close"))
            toggle_help();

        ImGui::End();
    }
}

void UI::help(const std::string& text)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void UI::input_int(const char* label, int& value, bool* value_changed, const int small_inc, const int big_inc, const int min, const int max)
{
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10);

    if (ImGui::InputInt(label, &value, small_inc, big_inc)) {
        value = std::clamp(value, min, max);

        if (value_changed)
            *value_changed = true;
    }

    ImGui::SameLine();
    help(fmt::format("{} to {}\n\n     -/+ to change by {}\nCTRL -/+ to change by {}", min, max, small_inc, big_inc));
}

void UI::input_double(const char* label, double& value, bool* value_changed, const double small_inc, const double big_inc, const double min, const double max)
{
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10);

    if (ImGui::InputDouble(label, &value, small_inc, big_inc)) {
        value = std::clamp(value, min, max);

        if (value_changed)
            *value_changed = true;
    }

    ImGui::SameLine();
    help(fmt::format("{} to {}\n\n     -/+ to change by {}\nCTRL -/+ to change by {}", min, max, small_inc, big_inc));
}

void UI::calculate_image(App& app)
{
    render_stopwatch_.start();
    app.calculate_image(supervisor_image_request_);
}
