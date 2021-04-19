#include "worker.h"

#include <mutex>
#include <thread>

#include <spdlog/spdlog.h>

#include "mandelbrot.h"

std::mutex mtx;

void worker_resize_iterations_histogram_if_needed(const WorkerCalculate& calculate, std::vector<int>& iterations_histogram)
{
    if (std::ssize(iterations_histogram) != calculate.max_iterations + 1)
        iterations_histogram.resize(static_cast<std::size_t>(calculate.max_iterations + 1));
}

void worker_combine_iterations_histogram(const std::vector<int>& iterations_histogram, std::vector<int>& combined_iterations_histogram)
{
    std::transform(iterations_histogram.cbegin(), iterations_histogram.cend(),
        combined_iterations_histogram.cbegin(), combined_iterations_histogram.begin(), std::plus<>{});
}

sf::Uint8 worker_calculation_result_to_color(const CalculationResult& point, const float log_max_iterations)
{
    return static_cast<sf::Uint8>(255.0f - 255.0f * std::log(static_cast<float>(point.iter)) / log_max_iterations);
}

void worker_draw_pixels(const WorkerCalculate& calculate)
{
    const float log_max_iterations = std::log(static_cast<float>(calculate.max_iterations));
    auto p = calculate.pixels.get();

    for (int y = 0; y < calculate.area.height; ++y) {
        for (int x = 0; x < calculate.area.width; ++x) {
            const std::size_t point = static_cast<std::size_t>((y + calculate.area.y) * calculate.image_size.width + (x + calculate.area.x));
            const auto color = worker_calculation_result_to_color((*calculate.results_per_point)[point], log_max_iterations);
            *p++ = color;
            *p++ = color;
            *p++ = color;
            *p++ = 255;
        }
    }
}

void worker(const int id, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue)
{
    spdlog::debug("worker {}: started", id);

    std::vector<int> iterations_histogram;

    while (true) {
        WorkerMessage msg = worker_message_queue.wait_for_message();

        if (std::holds_alternative<WorkerQuit>(msg)) {
            break;
        } else if (std::holds_alternative<WorkerCalculate>(msg)) {
            WorkerCalculate calculate = std::move(std::get<WorkerCalculate>(msg));
            worker_resize_iterations_histogram_if_needed(calculate, iterations_histogram);
            mandelbrot_calc(calculate.image_size, calculate.fractal_section, calculate.max_iterations, iterations_histogram, *calculate.results_per_point, calculate.area);
            worker_draw_pixels(calculate);

            {
                std::lock_guard<std::mutex> lock(mtx);
                worker_combine_iterations_histogram(iterations_histogram, *calculate.combined_iterations_histogram);
            }

            supervisor_message_queue.send(SupervisorCalculationResults{calculate.max_iterations, calculate.image_size, calculate.area, calculate.fractal_section, calculate.results_per_point, std::move(calculate.pixels)});
        } else if (std::holds_alternative<WorkerColorize>(msg)) {
            WorkerColorize colorize = std::move(std::get<WorkerColorize>(msg));
            mandelbrot_colorize(colorize);
            supervisor_message_queue.send(SupervisorColorizationResults{colorize.start_row, colorize.num_rows, colorize.row_width, colorize.colorization_buffer});
        }
    }

    spdlog::debug("worker {}: stopping", id);
}
