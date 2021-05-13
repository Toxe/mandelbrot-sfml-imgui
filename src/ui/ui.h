#pragma once

#include <string>
#include <vector>

#include <imgui.h>

#include "input_value.h"
#include "interface_hidden_hint_window.h"
#include "window.h"
#include "clock/duration.h"
#include "supervisor/phase.h"

class App;
class CommandLine;

class UI {
    const char* main_window_title_ = "Mandelbrot";
    const char* help_window_title_ = "Help";

    ImVec2 main_window_size_;

    InputValue<int> num_threads_;
    InputValue<int> max_iterations_;
    InputValue<int> tile_size_;
    InputValue<double> center_x_;
    InputValue<double> center_y_;
    InputValue<double> fractal_height_;

    float font_size_;

    bool is_visible_ = true;
    bool show_help_ = false;

    bool needs_to_recalculate_image_ = false;

    InterfaceHiddenHintWindow interface_hidden_hint_window_;

    std::vector<Gradient> available_gradients_;
    int selected_gradient_ = -1;

    void help(const std::string& text);
    bool input_int(const char* label, InputValue<int>& value, const int small_inc, const int big_inc, const int min, const int max);
    bool input_double(const char* label, InputValue<double>& value, const double small_inc, const double big_inc, const double min, const double max);

    void render_main_window(App& app);
    void render_help_window();
    void render_interface_hidden_hint_window();

    void reset_image_request_input_values_to_default();
    [[nodiscard]] bool image_request_input_values_have_changed();

    void show_status(const Phase phase);
    void show_render_time(const bool calculation_running, const Duration calculation_time);
    void show_gradient_selection(App& app);

public:
    UI(const CommandLine& cli);

    void render(App& app);

    void toggle_visibility();
    void toggle_help() { show_help_ = !show_help_; };

    void calculate_image(App& app);

    void scroll_image(App& app, int delta_x, int delta_y);
    void zoom_image(App& app, double factor);
};
