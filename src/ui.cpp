#include "ui.h"

#include <algorithm>
#include <atomic>
#include <limits>

#include <fmt/core.h>
#include <imgui.h>

#include "app.h"
#include "cli.h"
#include "messages.h"
#include "phase.h"
#include "supervisor.h"

const int default_max_iterations = 5000;
const int default_area_size = 100;
const FractalSection default_fractal_section = {-0.8, 0.0, 2.0};

UI::UI(const CLI& cli)
    : num_threads_{cli.num_threads()},
    font_size_{static_cast<float>(cli.font_size())}
{
    reset_image_request_input_values_to_default();
}

void UI::reset_image_request_input_values_to_default()
{
    max_iterations_.reset(default_max_iterations);
    area_size_.reset(default_area_size);
    center_x_.reset(default_fractal_section.center_x);
    center_y_.reset(default_fractal_section.center_y);
    fractal_height_.reset(default_fractal_section.height);
}

[[nodiscard]] bool UI::image_request_input_values_have_changed()
{
    return max_iterations_.changed() || area_size_.changed() || center_x_.changed() || center_y_.changed() || fractal_height_.changed();
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

    input_int("number of threads", num_threads_, 1, 10, 1, 1'000);

    if (num_threads_.changed() && phase == Phase::Idle) {
        if (ImGui::Button("change")) {
            app.change_num_threads(num_threads_.get());
            num_threads_.changed(false);
        }
    }

    ImGui::NewLine();

    input_double("center_x", center_x_, 0.1, 1.0, -5.0, 5.0);
    input_double("center_y", center_y_, 0.1, 1.0, -5.0, 5.0);
    input_double("fractal height", fractal_height_, 0.1, 1.0, 1000.0 * std::numeric_limits<double>::min(), 10.0);
    input_int("iterations", max_iterations_, 100, 1000, 10, 1'000'000);
    input_int("tile size", area_size_, 100, 500, 10, 10'000);

    if (phase == Phase::Idle) {
        if (ImGui::Button("Calculate"))
            calculate_image(app);

        if (image_request_input_values_have_changed()) {
            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                reset_image_request_input_values_to_default();
                calculate_image(app);
            }
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

void UI::input_int(const char* label, InputValue<int>& value, const int small_inc, const int big_inc, const int min, const int max)
{
    int val = value.get();

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 15);

    if (ImGui::InputInt(label, &val, small_inc, big_inc))
        value.set(std::clamp(val, min, max));

    ImGui::SameLine();
    help(fmt::format("{} to {}\n\n     -/+ to change by {}\nCTRL -/+ to change by {}", min, max, small_inc, big_inc));
}

void UI::input_double(const char* label, InputValue<double>& value, const double small_inc, const double big_inc, const double min, const double max)
{
    double val = value.get();

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 15);

    if (ImGui::InputDouble(label, &val, small_inc, big_inc, "%.16lf"))
        value.set(std::clamp(val, min, max));

    ImGui::SameLine();
    help(fmt::format("{} to {}\n\n     -/+ to change by {}\nCTRL -/+ to change by {}", min, max, small_inc, big_inc));
}

void UI::calculate_image(App& app)
{
    render_stopwatch_.start();
    app.calculate_image(SupervisorImageRequest{max_iterations_.get(), area_size_.get(), {0, 0}, {center_x_.get(), center_y_.get(), fractal_height_.get()}});
}
