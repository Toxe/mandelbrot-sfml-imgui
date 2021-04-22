#pragma once

#include <string>

#include "messages.h"
#include "stopwatch.h"
#include "window.h"

class App;
class CLI;

class UI {
    SupervisorImageRequest supervisor_image_request_;
    Stopwatch render_stopwatch_;
    float font_size_;
    bool is_visible_ = true;
    bool show_help_ = false;

    void help(const std::string& text);
    void input_int(const char* label, int& value, const int small_inc, const int big_inc, const int min, const int max);
    void input_double(const char* label, double& value, const double small_inc, const double big_inc, const double min, const double max);

    void render_main_window(App& app);
    void render_help_window();

public:
    UI(const CLI& cli);

    void render(App& app);

    void toggle_visibility() { is_visible_ = !is_visible_; };
    void toggle_help() { show_help_ = !show_help_; };

    SupervisorImageRequest make_default_supervisor_image_request();

    void calculate_image(App& app);
};
