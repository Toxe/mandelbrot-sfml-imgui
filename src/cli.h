#pragma once

#include <string_view>

#include <clipp.h>

class CLI {
    bool fullscreen_;
    int num_threads_;
    int font_size_;

    void show_usage_and_exit(const clipp::group& cli, const std::string_view& argv0, const std::string_view& description);

public:
    CLI(int argc, char* argv[]);

    bool fullscreen() const { return fullscreen_; }
    int num_threads() const { return num_threads_; }
    int font_size() const { return font_size_; }
};
