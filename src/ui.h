#pragma once

#include "app.h"
#include "cli.h"
#include "messages.h"
#include "stopwatch.h"

class UI {
    SupervisorImageRequest supervisor_image_request_;
    Stopwatch render_stopwatch_;
    float font_size_;
    bool is_visible_;

public:
    UI(const App& app, const CLI& cli);

    void shutdown();
    void render(const App& app);

    void toggle_visibility() { is_visible_ = !is_visible_; };

    SupervisorImageRequest make_default_supervisor_image_request(const App& app);
};
