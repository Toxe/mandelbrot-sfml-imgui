#pragma once

#include <string>
#include <vector>

#include <imgui.h>

#include "input_value.h"
#include "clock/duration.h"
#include "event_handler/event_handler.h"
#include "interface_hidden_hint_window.h"
#include "supervisor/phase.h"
#include "window/window.h"

class App;
class CommandLine;

class UI {
    const char* main_window_title_ = "Mandelbrot";
    const char* help_window_title_ = "Help";

    EventHandler* event_handler_ = nullptr;

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

    bool needs_to_recalculate_image_ = true;

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
    void show_gradient_selection();

public:
    UI(const CommandLine& cli);

    void render(App& app);

    void toggle_visibility();
    void toggle_help() { show_help_ = !show_help_; };

    int num_threads_input_value() { return num_threads_.get(); }
    void set_num_threads_input_value_changed(bool changed) { num_threads_.changed(changed); };

    void set_needs_to_recalculate_image(bool f) { needs_to_recalculate_image_ = f; };

    SupervisorImageRequest calculate_image_params(const ImageSize image_size);
    SupervisorImageRequest scroll_image_params(const ImageSize image_size, const int delta_x, const int delta_y);
    SupervisorImageRequest zoom_image_params(const ImageSize image_size, double factor);
    SupervisorColorize colorize_image_params(const ImageSize image_size);

    void set_event_handler(EventHandler* event_handler) { event_handler_ = event_handler; };
};
