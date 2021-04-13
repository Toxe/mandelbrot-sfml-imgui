#pragma once

#include <future>

#include <SFML/Graphics.hpp>

#include "messages.h"

enum class Phase {
    Starting,
    Idle,
    RequestSent,
    RequestReceived,
    Waiting,
    Coloring,
    Shutdown,
};

std::future<void> supervisor_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const Gradient& gradient);
void supervisor_stop();
void supervisor_calc_image(const SupervisorImageRequest& image_request);
const char* supervisor_phase_name(const Phase p);
