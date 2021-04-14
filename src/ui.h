#pragma once

#include <SFML/Graphics.hpp>

#include "app.h"
#include "messages.h"
#include "stopwatch.h"

class UI {
    SupervisorImageRequest supervisor_image_request_;
    Stopwatch render_stopwatch_;

public:
    UI(const App& app);

    void shutdown();
    void render(const App& app, sf::Clock& clock, sf::Image& image);

    SupervisorImageRequest make_default_supervisor_image_request(const App& app);
};
