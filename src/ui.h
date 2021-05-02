#pragma once

#include <string>
#include <vector>

#include <imgui.h>

#include "duration.h"
#include "input_value.h"
#include "phase.h"
#include "window.h"

class App;
class CommandLine;

class UI {
    const char* main_window_title_ = "Mandelbrot";
    const char* help_window_title_ = "Help";

    ImVec2 main_window_size_;

    InputValue<int> num_threads_;
    InputValue<int> max_iterations_;
    InputValue<int> area_size_;
    InputValue<double> center_x_;
    InputValue<double> center_y_;
    InputValue<double> fractal_height_;

    float font_size_;

    bool is_visible_ = true;
    bool show_help_ = false;

    std::vector<Gradient> available_gradients_;
    int selected_gradient_ = -1;

    void help(const std::string& text);
    void input_int(const char* label, InputValue<int>& value, const int small_inc, const int big_inc, const int min, const int max);
    void input_double(const char* label, InputValue<double>& value, const double small_inc, const double big_inc, const double min, const double max);

    void render_main_window(App& app);
    void render_help_window();

    void reset_image_request_input_values_to_default();
    [[nodiscard]] bool image_request_input_values_have_changed();

    void show_status(const Phase phase);
    void show_render_time(const bool calculation_running, const Duration calculation_time);
    void show_gradient_selection(App& app);

public:
    UI(const CommandLine& cli);

    void render(App& app);

    void toggle_visibility() { is_visible_ = !is_visible_; };
    void toggle_help() { show_help_ = !show_help_; };

    void calculate_image(App& app);
};
