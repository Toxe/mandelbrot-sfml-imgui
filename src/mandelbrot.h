#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "gradient.h"

struct CalculationResult {
    int iter;
    float distance_to_next_iteration;
};

struct ImageSize {
    int width, height;
};

struct Section {
    double center_x, center_y, height;
};

void mandelbrot_calc(const ImageSize& image, const Section& section, const int max_iterations,
                     std::vector<int>& iterations_histogram, std::vector<CalculationResult>& results_per_point,
                     const int start_x, const int start_y, const int work_size) noexcept;
void mandelbrot_colorize(const int max_iterations, const Gradient& gradient,
                         sf::Image& image, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point) noexcept;
