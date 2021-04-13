#pragma once

#include <future>

#include <SFML/Graphics.hpp>

#include "messages.h"

std::future<void> supervisor_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient);
void supervisor_stop();
void supervisor_calc_image(const SupervisorImageRequest& image_request);
