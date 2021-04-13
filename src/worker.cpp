#include "worker.h"

#include <thread>

#include <spdlog/spdlog.h>

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

            if (std::ssize(iterations_histogram) != calc.max_iterations + 1) {
                spdlog::debug("worker {}: iterations_histogram {} --> {}", id, iterations_histogram.size(), calc.max_iterations + 1);
                iterations_histogram = std::vector<int>(static_cast<std::size_t>(calc.max_iterations + 1));
            }

            mandelbrot_calc(calc.image_size, calc.fractal_section, calc.max_iterations, iterations_histogram, *calc.results_per_point, calc.area);

            {
                std::lock_guard<std::mutex> lock(mtx);
                std::transform(iterations_histogram.cbegin(), iterations_histogram.cend(), calc.combined_iterations_histogram->cbegin(), calc.combined_iterations_histogram->begin(), std::plus<>{});
                supervisor_message_queue.push(SupervisorResultsFromWorker{calc.max_iterations, calc.image_size, calc.area, calc.fractal_section, calc.results_per_point});
            }
        }

        // signal other workers or the supervisor
        cv.notify_one();
    }

    spdlog::debug("worker {}: stopping", id);
}
