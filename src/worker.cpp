#include "worker.h"

#include <thread>

#include <spdlog/spdlog.h>

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

WorkerMessage worker_wait_for_message(std::mutex& mtx, std::condition_variable& cv, std::queue<WorkerMessage>& message_queue)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return !message_queue.empty(); });

    const WorkerMessage msg = message_queue.front();
    message_queue.pop();

    return msg;
}

void worker(const int id, std::mutex& mtx, std::condition_variable& cv_wk, std::condition_variable& cv_sv, std::queue<WorkerMessage>& worker_message_queue, std::queue<SupervisorMessage>& supervisor_message_queue)
{
    spdlog::debug("worker {}: started", id);

    std::vector<int> iterations_histogram;

    while (true) {
        const WorkerMessage msg = worker_wait_for_message(mtx, cv_wk, worker_message_queue);

        if (std::holds_alternative<WorkerQuit>(msg)) {
            spdlog::debug("worker {}: quit", id);
            break;
        } else if (std::holds_alternative<WorkerCalc>(msg)) {
            WorkerCalc calc{std::get<WorkerCalc>(msg)};
            worker_resize_iterations_histogram_if_needed(calc, iterations_histogram);
            mandelbrot_calc(calc.image_size, calc.fractal_section, calc.max_iterations, iterations_histogram, *calc.results_per_point, calc.area);

            {
                std::lock_guard<std::mutex> lock(mtx);
                worker_combine_iterations_histogram(iterations_histogram, *calc.combined_iterations_histogram);
                supervisor_message_queue.push(SupervisorResultsFromWorker{calc.max_iterations, calc.image_size, calc.area, calc.fractal_section, calc.results_per_point});
            }
        }

        cv_sv.notify_one();
    }

    spdlog::debug("worker {}: stopping", id);
}
