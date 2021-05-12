#include "ui.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "app.h"
#include "colors.h"
#include "command_line/command_line.h"
#include "gradient/gradient.h"
#include "messages/messages.h"
#include "supervisor/supervisor.h"

const int default_max_iterations = 5000;
const int default_area_size = 100;
const FractalSection default_fractal_section = {-0.8, 0.0, 2.0};

UI::UI(const CommandLine& cli)
    : num_threads_{cli.num_threads()},
    font_size_{static_cast<float>(cli.font_size())}
{
    reset_image_request_input_values_to_default();
    available_gradients_ = load_available_gradients();
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
    render_interface_hidden_hint_window();
}

void UI::render_main_window(App& app)
{
    static std::vector<float> fps(120);
    static std::size_t values_offset = 0;

    const Phase phase = app.supervisor_status().phase();
    const bool calculation_running = app.supervisor_status().calculation_running();
    const Duration calculation_time = app.supervisor_status().calculation_time();

    const float elapsed_time_in_seconds = app.elapsed_time().as_seconds();
    const float current_fps = app.elapsed_time().fps();
    const auto fps_label = fmt::format("{:.1f} FPS ({:.3f} ms/frame)", current_fps, 1000.0f * elapsed_time_in_seconds);
    fps[values_offset] = current_fps;
    values_offset = (values_offset + 1) % fps.size();

    if (!is_visible_)
        return;

    const auto window_size = app.window().size();

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin(main_window_title_, nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::PlotLines("", fps.data(), static_cast<int>(fps.size()), static_cast<int>(values_offset), fps_label.c_str(), 0.0f, 1.5f * std::max(65.0f, *std::max_element(fps.begin(), fps.end())), ImVec2(0, 4.0f * font_size_));

    ImGui::TextColored(UserInterface::Colors::light_gray, "image size:");
    ImGui::SameLine();
    ImGui::Text("%dx%d", window_size.width, window_size.height);

    show_status(phase);
    show_render_time(calculation_running, calculation_time);

    if (ImGui::Button("Help (F1)"))
        toggle_help();

    ImGui::SameLine();

    if (ImGui::Button("Fullscreen (F10)"))
        app.window().toggle_fullscreen();

    input_int("number of threads", num_threads_, 1, 10, 1, 1'000);

    if (num_threads_.changed()) {
        if (phase == Phase::Idle) {
            if (ImGui::Button("change")) {
                app.change_num_threads(num_threads_.get());
                num_threads_.changed(false);
            }
        } else {
            ImGui::TextDisabled("waiting for calculation to finish...");
        }
    }

    ImGui::NewLine();

    input_double("center_x", center_x_, 0.1, 1.0, -5.0, 5.0);
    input_double("center_y", center_y_, 0.1, 1.0, -5.0, 5.0);
    input_double("fractal height", fractal_height_, 0.1, 1.0, 1000.0 * std::numeric_limits<double>::min(), 10.0);
    input_int("iterations", max_iterations_, 1000, 10000, 10, 1'000'000);
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

        if (ImGui::Button("left")) scroll_image(app, -100, 0); ImGui::SameLine();
        if (ImGui::Button("right")) scroll_image(app, 100, 0); ImGui::SameLine();
        if (ImGui::Button("up")) scroll_image(app, 0, -100); ImGui::SameLine();
        if (ImGui::Button("down")) scroll_image(app, 0, 100);
        // if (ImGui::Button("left/up")) scroll_image(app, -100, -100); ImGui::SameLine();
        // if (ImGui::Button("right/up")) scroll_image(app, 100, -100); ImGui::SameLine();
        // if (ImGui::Button("left/down")) scroll_image(app, -100, 100); ImGui::SameLine();
        // if (ImGui::Button("right/down")) scroll_image(app, 100, 100);
    }

    if (calculation_running) {
        if (phase == Phase::Calculating) {
            if (ImGui::Button("Cancel"))
                app.cancel_calculation();
        } else if (phase == Phase::Coloring) {
            ImGui::TextDisabled("waiting for colorization to finish...");
        } else if (phase == Phase::Canceled) {
            ImGui::TextDisabled("waiting for calculation to finish...");
        } else {
            ImGui::TextDisabled("waiting...");
        }
    }

    show_gradient_selection(app);

    main_window_size_ = ImGui::GetWindowSize();
    ImGui::End();
}

void UI::render_help_window()
{
    if (show_help_) {
        ImGui::SetNextWindowPos(ImVec2(20 + 20 + main_window_size_.x, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin(help_window_title_, &show_help_, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(UserInterface::Colors::light_blue, "Left/right click");
        ImGui::SameLine();
        ImGui::Text("zoom in/out");

        ImGui::TextColored(UserInterface::Colors::light_blue, "       Left drag");
        ImGui::SameLine();
        ImGui::Text("zoom in area");

        ImGui::TextColored(UserInterface::Colors::light_blue, "      Right drag");
        ImGui::SameLine();
        ImGui::Text("move around");

        ImGui::Separator();

        ImGui::TextColored(UserInterface::Colors::light_blue, "Enter");
        ImGui::SameLine();
        ImGui::Text("calculate image");

        ImGui::TextColored(UserInterface::Colors::light_blue, "Space");
        ImGui::SameLine();
        ImGui::Text("show/hide UI");

        ImGui::TextColored(UserInterface::Colors::light_blue, "   F1");
        ImGui::SameLine();
        ImGui::Text("show/hide help");

        ImGui::TextColored(UserInterface::Colors::light_blue, "  F10");
        ImGui::SameLine();
        ImGui::Text("fullscreen");

        ImGui::TextColored(UserInterface::Colors::light_blue, "  ESC");
        ImGui::SameLine();
        ImGui::Text("quit");

        if (ImGui::Button("Close"))
            toggle_help();

        ImGui::End();
    }
}

void UI::render_interface_hidden_hint_window()
{
    if (interface_hidden_hint_window_.visible())
        interface_hidden_hint_window_.render();
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
    const auto image_size = app.window().size();
    const auto calculation_area = CalculationArea{0, 0, image_size.width, image_size.height};
    app.calculate_image(SupervisorImageRequest{max_iterations_.get(), area_size_.get(), image_size, calculation_area, {0, 0}, {center_x_.get(), center_y_.get(), fractal_height_.get()}});
}

void UI::scroll_image(App& app, int delta_x, int delta_y)
{
    assert((delta_x != 0 && delta_y == 0) || (delta_x == 0 && delta_y != 0));

    spdlog::debug("scroll {}/{}", delta_x, delta_y);

    const ImageSize image_size = app.window().size();
    CalculationArea calculation_area;
    FractalSection fractal_section;
    Scroll scroll{delta_x, delta_y};

    double center_x = center_x_.get();
    double center_y = center_y_.get();
    double fractal_height = fractal_height_.get();
    double fractal_width = fractal_height * (static_cast<double>(image_size.width) / static_cast<double>(image_size.height));

    if (delta_x != 0) {
        double fractal_delta = fractal_width * static_cast<double>(std::abs(delta_x)) / static_cast<double>(image_size.width);

        if (delta_x < 0) {
            calculation_area = CalculationArea{0, 0, -delta_x, image_size.height};
            fractal_section = FractalSection{center_x - fractal_delta, center_y, fractal_height};
        } else {
            calculation_area = CalculationArea{image_size.width - delta_x, 0, delta_x, image_size.height};
            fractal_section = FractalSection{center_x + fractal_delta, center_y, fractal_height};
        }
    } else {
        // be aware that the y axis of the fractal coordinate system runs in the opposite direction of the screen y coordinates
        double fractal_delta = fractal_height * static_cast<double>(std::abs(delta_y)) / static_cast<double>(image_size.height);

        if (delta_y < 0) {
            calculation_area = CalculationArea{0, 0, image_size.width, -delta_y};
            fractal_section = FractalSection{center_x, center_y + fractal_delta, fractal_height};
        } else {
            calculation_area = CalculationArea{0, image_size.height - delta_y, image_size.width, delta_y};
            fractal_section = FractalSection{center_x, center_y - fractal_delta, fractal_height};
        }
    }

    center_x_.set(fractal_section.center_x);
    center_y_.set(fractal_section.center_y);

    app.calculate_image(SupervisorImageRequest{max_iterations_.get(), area_size_.get(), image_size, calculation_area, scroll, fractal_section});
}

void UI::show_status(const Phase phase)
{
    static Clock clock;
    ImVec4 phase_color;

    if (phase == Phase::Idle) {
        phase_color = UserInterface::Colors::phase_idle;
    } else if (phase == Phase::Canceled) {
        phase_color = UserInterface::Colors::phase_canceled;
    } else if (phase == Phase::Calculating) {
        double itgr;
        double rmdr = std::modf(clock.elapsed_time().as_seconds(), &itgr);
        float f = static_cast<float>((2.0 / 3.0) + std::sin(rmdr * 2.0 * std::numbers::pi) / 3.0);
        phase_color = ImVec4{f, f, f, 1.0f};
    } else if (phase == Phase::Coloring) {
        float r, g, b;
        float itgr;
        float rmdr = std::modf(clock.elapsed_time().as_seconds(), &itgr);
        ImGui::ColorConvertHSVtoRGB(rmdr, 1.0f, 1.0f, r, g, b);
        phase_color = ImVec4{r, g, b, 1.0f};
    } else {
        phase_color = UserInterface::Colors::phase_default;
    }

    ImGui::TextColored(UserInterface::Colors::light_gray, "status:");
    ImGui::SameLine();
    ImGui::TextColored(phase_color, "%s", phase_name(phase));
}

void UI::show_render_time(bool calculation_running, Duration calculation_time)
{
    ImGui::TextColored(UserInterface::Colors::light_gray, "render time:");
    ImGui::SameLine();

    if (calculation_running)
        ImGui::TextColored(UserInterface::Colors::yellow, "%.3fs", calculation_time.as_seconds());
    else
        ImGui::Text("%.3fs", calculation_time.as_seconds());
}

void UI::show_gradient_selection(App& app)
{
    ImGui::NewLine();
    ImGui::Text("Colors");

    ImGui::BeginChild("gradient selection", ImVec2(0, font_size_ * 10), true);

    for (int i = 0; i < std::ssize(available_gradients_); ++i) {
        const auto& gradient = available_gradients_[static_cast<std::size_t>(i)];

        if (ImGui::Selectable(gradient.name_.c_str(), selected_gradient_ == i)) {
            selected_gradient_ = i;
            app.colorize(SupervisorColorize{max_iterations_.get(), {0, 0}, gradient});
        }
    }

    ImGui::EndChild();
}

void UI::toggle_visibility()
{
    is_visible_ = !is_visible_;

    if (!is_visible_)
        interface_hidden_hint_window_.show();
    else
        interface_hidden_hint_window_.hide();
};
