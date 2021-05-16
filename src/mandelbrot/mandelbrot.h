#pragma once

#include <vector>

#include "gradient/gradient.h"
#include "messages/messages.h"

void mandelbrot_calc(const ImageSize& image, const FractalSection& section, const int max_iterations,
                     std::vector<CalculationResult>& results_per_point, const CalculationArea& area) noexcept;
void mandelbrot_colorize(WorkerColorize& colorize) noexcept;
void equalize_histogram(const std::vector<int>& iterations_histogram, const int max_iterations, std::vector<float>& equalized_iterations);
