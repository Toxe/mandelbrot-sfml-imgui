#pragma once

#include <chrono>

using namespace std::chrono_literals;

class InterfaceHiddenHintWindow {
    std::chrono::steady_clock::time_point tp_window_shown_;

    bool visible_ = false;
    bool has_been_shown_ = false;

public:
    [[nodiscard]] bool visible() const { return visible_; }

    void show();
    void hide();

    void render();
};
