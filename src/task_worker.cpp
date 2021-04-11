#include "task_worker.h"

#include <thread>

#include <spdlog/spdlog.h>

extern bool workers_running;

void task_worker(const int id, std::mutex& mtx, std::condition_variable& cv, std::queue<WorkUnit>& work_queue, std::queue<WorkResult>& results_queue)
{
    spdlog::debug("worker {}: started", id);

    while (true) {
        WorkUnit work;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return !work_queue.empty() || !workers_running; });

            if (!workers_running)
                break;

            work = work_queue.front();
            work_queue.pop();
            // spdlog::debug("worker {}: received work row: {}, column: {} (queue size: {})", id, work.start_y, work.start_x, work_queue.size());
        }

        std::vector<int> iterations_histogram(static_cast<std::size_t>(work.request.max_iterations + 1), 0);
        mandelbrot_calc(work.request.image_size, work.request.section, work.request.max_iterations, iterations_histogram, *work.results_per_point, work.start_x, work.start_y, work.size);

        {
            std::lock_guard<std::mutex> lock(mtx);
            std::transform(iterations_histogram.cbegin(), iterations_histogram.cend(), work.combined_iterations_histogram->cbegin(), work.combined_iterations_histogram->begin(), std::plus<>{});
            results_queue.push(WorkResult{work});
        }

        // signal other workers or the task master
        cv.notify_one();
    }

    spdlog::debug("worker {}: stopping", id);
}
