#include "task_master.h"

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

#include "mandelbrot.h"
#include "task_worker.h"

std::vector<std::thread> workers;
std::condition_variable cv;
std::mutex mtx;
std::mutex paint_mtx;

std::queue<ImageRequest> requests_queue;
std::queue<WorkUnit> work_queue;
std::queue<WorkResult> results_queue;

bool task_master_running = true;
bool workers_running = true;

std::atomic<bool> task_master_is_working = true;

void task_master_colorize(sf::Image& image, sf::Texture& texture, const int max_iterations, const Gradient& gradient, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point)
{
    mandelbrot_colorize(max_iterations, gradient, image, iterations_histogram, results_per_point);

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void task_master_clear_image(const ImageSize& image_size, sf::Image& image, sf::Texture& texture)
{
    for (int y = 0; y < image_size.height; ++y)
        for (int x = 0; x < image_size.width; ++x)
            image.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), sf::Color::Blue);

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void task_master_create_work(const ImageRequest& request, std::vector<int>& combined_iterations_histogram, std::vector<CalculationResult>& results_per_point)
{
    const int size = 100;

    std::lock_guard<std::mutex> lock(mtx);

    for (int y = 0; y < (request.image_size.height / size); ++y) {
        for (int x = 0; x < (request.image_size.width / size); ++x) {
            // spdlog::debug("task master: providing work posx={}, posy={}", x * size, y * size);
            work_queue.push(WorkUnit{x * size, y * size, size, request, &combined_iterations_histogram, &results_per_point});
        }
    }
}

void task_master_receive_result(const WorkResult& result, sf::Image& image, sf::Texture& texture)
{
    // spdlog::debug("task master: received result x={}..{}, y={}..{}", result.work.start_x, result.work.start_x + result.work.size, result.work.start_y, result.work.start_y + result.work.size);

    const float max = std::log(static_cast<float>(result.work.request.max_iterations));

    for (int y = result.work.start_y; y < (result.work.start_y + result.work.size); ++y) {
        for (int x = result.work.start_x; x < (result.work.start_x + result.work.size); ++x) {
            std::size_t pixel = static_cast<std::size_t>(y * result.work.request.image_size.width + x);
            const CalculationResult& rpp{(*result.work.results_per_point)[pixel]};
            const auto rgb = static_cast<sf::Uint8>(255.0f - 255.0f * std::log(static_cast<float>(rpp.iter)) / max);
            const auto color = sf::Color(rgb, rgb, rgb);
            image.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), color);
            ++pixel;
        }
    }

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void task_master(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient)
{
    spdlog::debug("task master: starting");

    std::vector<int> combined_iterations_histogram(static_cast<std::size_t>(max_iterations + 1));
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(image.getSize().x * image.getSize().y));

    for (unsigned int id = 0; id < num_threads; ++id)
        workers.emplace_back(task_worker, id, std::ref(mtx), std::ref(cv), std::ref(work_queue), std::ref(results_queue));

    task_master_is_working = false;

    while (true) {
        int todo = 0; // 1: receive request, 2: receive result
        ImageRequest request;
        WorkResult result;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return !requests_queue.empty() || !results_queue.empty() || !task_master_running; });

            if (!task_master_running) {
                task_master_is_working = true;
                break;
            }

            if (!requests_queue.empty()) {
                request = requests_queue.front();
                requests_queue.pop();
                todo = 1;
            } else if (!results_queue.empty()) {
                result = results_queue.front();
                results_queue.pop();
                todo = 2;
            }
        }

        if (todo == 1) {
            spdlog::debug("task master: received image request");
            task_master_is_working = true;
            task_master_clear_image(request.image_size, image, texture);
            task_master_create_work(request, combined_iterations_histogram, results_per_point);
        } else if (todo == 2) {
            task_master_receive_result(result, image, texture);

            {
                std::lock_guard<std::mutex> lock(mtx);
                // spdlog::debug("task master: received result row: {}, column: {} (work_queue: {}, results_queue: {})", result.work.start_y, result.work.start_x, work_queue.size(), results_queue.size());

                // task_master_colorize(image, texture, result.work.request.max_iterations, gradient, combined_iterations_histogram, results_per_point);

                if (work_queue.empty() && results_queue.empty()) {
                    spdlog::debug("task_master_is_working --> false");
                    task_master_is_working = false;
                }
            }
        }

        cv.notify_one();
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        spdlog::debug("task master: signaling workers to stop");
        workers_running = false;
    }

    cv.notify_all();

    spdlog::debug("task master: waiting for workers to finish");

    for (auto& t : workers)
        t.join();

    spdlog::debug("task master: done");
}

std::future<void> task_master_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient)
{
    return std::async(std::launch::async, task_master, std::ref(image), std::ref(texture), num_threads, max_iterations, gradient);
}

void task_master_stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    spdlog::debug("task_master_stop");
    task_master_running = false;
    cv.notify_all();
}

void task_master_image_request(const ImageRequest& request)
{
    std::lock_guard<std::mutex> lock(mtx);
    spdlog::debug("image request");
    requests_queue.push(request);
    cv.notify_all();
}

bool task_master_working()
{
    return task_master_is_working;
}
