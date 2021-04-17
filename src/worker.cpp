#include "worker.h"

#include <mutex>
#include <thread>

#include <spdlog/spdlog.h>

#include "util/mutex_timer.h"

extern MutexTimer mutex_timer_worker_combine_iterations_histogram;

void worker_resize_iterations_histogram_if_needed(const WorkerCalc& calc, std::vector<int>& iterations_histogram)
{
    if (std::ssize(iterations_histogram) != calc.max_iterations + 1)
        iterations_histogram.resize(static_cast<std::size_t>(calc.max_iterations + 1));
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

void worker_draw_pixels(const WorkerCalc& calc)
{
    const float log_max_iterations = std::log(static_cast<float>(calc.max_iterations));
    auto p = calc.pixels.get();

    for (int y = 0; y < calc.area.height; ++y) {
        for (int x = 0; x < calc.area.width; ++x) {
            const std::size_t point = static_cast<std::size_t>((y + calc.area.y) * calc.image_size.width + (x + calc.area.x));
            const auto color = worker_calculation_result_to_color((*calc.results_per_point)[point], log_max_iterations);
            *p++ = color;
            *p++ = color;
            *p++ = color;
            *p++ = 255;
        }
    }
}

void worker(const int id, std::mutex& mtx, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue)
{
    spdlog::debug("worker {}: started", id);

    std::vector<int> iterations_histogram;

    while (true) {
        WorkerMessage msg = worker_message_queue.wait_for_message();

        if (std::holds_alternative<WorkerQuit>(msg)) {
            spdlog::debug("worker {}: quit", id);
            break;
        } else if (std::holds_alternative<WorkerCalc>(msg)) {
            WorkerCalc calc = std::move(std::get<WorkerCalc>(msg));
            worker_resize_iterations_histogram_if_needed(calc, iterations_histogram);
            mandelbrot_calc(calc.image_size, calc.fractal_section, calc.max_iterations, iterations_histogram, *calc.results_per_point, calc.area);
            worker_draw_pixels(calc);

            {
                const auto t0 = std::chrono::high_resolution_clock::now();
                std::lock_guard<std::mutex> lock(mtx);
                mutex_timer_worker_combine_iterations_histogram.update(t0);
                worker_combine_iterations_histogram(iterations_histogram, *calc.combined_iterations_histogram);
            }

            supervisor_message_queue.send(SupervisorResultsFromWorker{calc.max_iterations, calc.image_size, calc.area, calc.fractal_section, calc.results_per_point, std::move(calc.pixels)});
        }
    }

    spdlog::debug("worker {}: stopping", id);
}
