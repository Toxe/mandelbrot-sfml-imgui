#pragma once

#include <SFML/Graphics.hpp>

#include "app.h"
#include "cli.h"
#include "messages.h"
#include "stopwatch.h"

class UI {
    SupervisorImageRequest supervisor_image_request_;
    Stopwatch render_stopwatch_;

    sf::Clock frame_time_clock_;

public:
    UI(const App& app, const CLI& cli);

    void shutdown();
    void render(const App& app, sf::Image& image);

    SupervisorImageRequest make_default_supervisor_image_request(const App& app);
};
