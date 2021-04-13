#include "worker.h"

#include <thread>

#include <spdlog/spdlog.h>

void worker_resize_iterations_histogram_if_needed(const int id, const WorkerCalc& calc, std::vector<int>& iterations_histogram)
{
    if (std::ssize(iterations_histogram) != calc.max_iterations + 1) {
        spdlog::debug("worker {}: resize iterations_histogram {} --> {}", id, iterations_histogram.size(), calc.max_iterations + 1);
        iterations_histogram.resize(static_cast<std::size_t>(calc.max_iterations + 1));
    }
}

void worker_combine_iterations_histogram(const std::vector<int>& iterations_histogram, std::vector<int>& combined_iterations_histogram)
{
    std::transform(iterations_histogram.cbegin(), iterations_histogram.cend(),
        combined_iterations_histogram.cbegin(), combined_iterations_histogram.begin(), std::plus<>{});
}

void worker(const int id, std::mutex& mtx, std::condition_variable& cv, std::queue<WorkerMessage>& worker_message_queue, std::queue<SupervisorMessage>& supervisor_message_queue)
{
    spdlog::debug("worker {}: started", id);

    std::vector<int> iterations_histogram;

    while (true) {
        WorkerMessage msg;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return !worker_message_queue.empty(); });

            msg = worker_message_queue.front();
            worker_message_queue.pop();
        }

        if (std::holds_alternative<WorkerQuit>(msg)) {
            spdlog::debug("worker {}: received WorkerQuit", id);
            break;
        } else if (std::holds_alternative<WorkerCalc>(msg)) {
            WorkerCalc calc{std::get<WorkerCalc>(msg)};
            worker_resize_iterations_histogram_if_needed(id, calc, iterations_histogram);
            mandelbrot_calc(calc.image_size, calc.fractal_section, calc.max_iterations, iterations_histogram, *calc.results_per_point, calc.area);

            {
                std::lock_guard<std::mutex> lock(mtx);
                worker_combine_iterations_histogram(iterations_histogram, *calc.combined_iterations_histogram);
                supervisor_message_queue.push(SupervisorResultsFromWorker{calc.max_iterations, calc.image_size, calc.area, calc.fractal_section, calc.results_per_point});
            }
        }

        // signal other workers or the supervisor
        cv.notify_one();
    }

    spdlog::debug("worker {}: stopping", id);
}
