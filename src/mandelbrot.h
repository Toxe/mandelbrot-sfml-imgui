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

struct CalculationArea {
    int start_x, start_y;
    int size;
};

struct FractalSection {
    double center_x, center_y, height;
};

void mandelbrot_calc(const ImageSize& image, const FractalSection& section, const int max_iterations,
                     std::vector<int>& iterations_histogram, std::vector<CalculationResult>& results_per_point, const CalculationArea& area) noexcept;
void mandelbrot_colorize(const int max_iterations, const Gradient& gradient,
                         sf::Image& image, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point) noexcept;
