#pragma once

#include <future>
#include <vector>

#include <SFML/Graphics.hpp>

#include "mandelbrot.h"

struct ImageRequest {
    ImageSize image_size;
    Section section;
    int max_iterations;
};

struct WorkUnit {
    int start_x, start_y;
    int size;
    ImageRequest request;
    std::vector<int>* combined_iterations_histogram;
    std::vector<CalculationResult>* results_per_point;
};

struct WorkResult {
    WorkUnit work;
};

std::future<void> supervisor_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient);
void supervisor_stop();
void supervisor_image_request(const ImageRequest& request);
bool supervisor_working();
