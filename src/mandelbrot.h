#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "gradient.h"

struct CalculationResult {
    int iter;
    float distance_to_next_iteration;
};

void mandelbrot_calc(const int image_width, const int image_height, const int max_iterations, const double center_x, const double center_y, const double height,
                     std::vector<int>& iterations_histogram, std::vector<CalculationResult>& results_per_point) noexcept;
void mandelbrot_colorize(const int max_iterations, const Gradient& gradient,
                         sf::Image& image, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point) noexcept;
