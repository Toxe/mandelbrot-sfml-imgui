#include "mandelbrot.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

void mandelbrot_calc(const ImageSize& image, const FractalSection& section, const int max_iterations,
                     std::vector<int>& iterations_histogram, std::vector<CalculationResult>& results_per_point, const CalculationArea& area) noexcept
{
    const double width = section.height * (static_cast<double>(image.width) / static_cast<double>(image.height));

    const double x_left   = section.center_x - width / 2.0;
    const double x_right  = section.center_x + width / 2.0;
    const double y_top    = section.center_y + section.height / 2.0;
    const double y_bottom = section.center_y - section.height / 2.0;

    constexpr double bailout = 20.0;
    constexpr double bailout_squared = bailout * bailout;
    const double log_log_bailout = std::log(std::log(bailout));
    const double log_2 = std::log(2.0);

    double final_magnitude = 0.0;

    std::fill(iterations_histogram.begin(), iterations_histogram.end(), 0);

    for (int pixel_y = area.y; pixel_y < (area.y + area.height); ++pixel_y) {
        const double y0 = std::lerp(y_top, y_bottom, static_cast<double>(pixel_y) / static_cast<double>(image.height));

        for (int pixel_x = area.x; pixel_x < (area.x + area.width); ++pixel_x) {
            const double x0 = std::lerp(x_left, x_right, static_cast<double>(pixel_x) / static_cast<double>(image.width));

            double x = 0.0;
            double y = 0.0;

            // iteration, will be from 1 .. max_iterations once the loop is done
            int iter = 0;

            while (iter < max_iterations) {
                const double x_squared = x * x;
                const double y_squared = y * y;

                if (x_squared + y_squared >= bailout_squared) {
                    final_magnitude = std::sqrt(x_squared + y_squared);
                    break;
                }

                const double xtemp = x_squared - y_squared + x0;
                y = 2.0 * x * y + y0;
                x = xtemp;

                ++iter;
            }

            const std::size_t pixel = static_cast<std::size_t>(pixel_y * image.width + pixel_x);

            if (iter < max_iterations) {
                ++iterations_histogram[static_cast<std::size_t>(iter)]; // iter: 1 .. max_iterations-1, no need to count iterations_histogram[max_iterations]
                results_per_point[pixel] = CalculationResult{iter, 1.0f - std::min(1.0f, static_cast<float>((std::log(std::log(final_magnitude)) - log_log_bailout) / log_2))};
            } else {
                results_per_point[pixel] = CalculationResult{iter, 0.0};
            }
        }
    }
}

void equalize_histogram(const std::vector<int>& iterations_histogram, const int max_iterations, std::vector<float>& equalized_iterations)
{
    assert(iterations_histogram.size() == equalized_iterations.size());

    // Calculate the CDF (Cumulative Distribution Function) by accumulating all iteration counts.
    // Element [0] is unused and iterations_histogram[max_iterations] should be zero (as we do not count
    // the iterations of the points inside the Mandelbrot Set).
    std::vector<int> cdf(iterations_histogram.size());
    std::partial_sum(iterations_histogram.cbegin(), iterations_histogram.cend(), cdf.begin());

    // Get the minimum value in the CDF that is bigger than zero and the sum of all iteration counts
    // from iterations_histogram (which is the last value of the CDF).
    const auto cdf_min = std::find_if(cdf.cbegin(), cdf.cend(), [](auto n) { return n > 0; });
    const auto total_iterations = cdf[cdf.size() - 1];

    // normalize all values from the CDF that are bigger than zero to a range of 0.0 .. max_iterations
    const auto f = static_cast<float>(max_iterations) / static_cast<float>(total_iterations - *cdf_min);
    std::transform(cdf.cbegin(), cdf.cend(), equalized_iterations.begin(),
                   [=](const auto& c) { return c > 0 ? f * static_cast<float>(c - *cdf_min) : 0.0f; });
}

void mandelbrot_colorize(WorkerColorize& colorize) noexcept
{
    for (int y = colorize.start_row; y < (colorize.start_row + colorize.num_rows); ++y) {
        auto point = colorize.results_per_point->cbegin() + (y * colorize.row_width);

        for (int x = 0; x < colorize.row_width; ++x) {
            std::size_t p = static_cast<std::size_t>(4 * (y * colorize.row_width + x));

            if (point->iter == colorize.max_iterations) {
                // points inside the Mandelbrot Set are always painted black
                (*colorize.colorization_buffer)[p++] = 0;
                (*colorize.colorization_buffer)[p++] = 0;
                (*colorize.colorization_buffer)[p++] = 0;
                (*colorize.colorization_buffer)[p++] = 255;
            } else {
                // The equalized iteration value (in the range of 0 .. max_iterations) represents the
                // position of the pixel color in the color gradiant and needs to be mapped to 0.0 .. 1.0.
                // To achieve smooth coloring we need to edge the equalized iteration towards the next
                // iteration, determined by the distance between the two iterations.
                const auto iter_curr = (*colorize.equalized_iterations)[static_cast<std::size_t>(point->iter)];
                const auto iter_next = (*colorize.equalized_iterations)[static_cast<std::size_t>(point->iter + 1)];

                const auto smoothed_iteration = std::lerp(iter_curr, iter_next, point->distance_to_next_iteration);
                const auto pos_in_gradient = smoothed_iteration / static_cast<float>(colorize.max_iterations);

                const auto color = color_from_gradient(*colorize.gradient, pos_in_gradient);

                (*colorize.colorization_buffer)[p++] = color.r;
                (*colorize.colorization_buffer)[p++] = color.g;
                (*colorize.colorization_buffer)[p++] = color.b;
                (*colorize.colorization_buffer)[p++] = color.a;
            }

            ++point;
        }
    }
}
