#include "supervisor.h"

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

#include "worker.h"

std::vector<std::thread> workers;
std::condition_variable cv;
std::mutex mtx;
std::mutex paint_mtx;

std::queue<SupervisorMessage> supervisor_message_queue;
std::queue<WorkerMessage> worker_message_queue;

void supervisor_colorize(sf::Image& image, sf::Texture& texture, const int max_iterations, const Gradient& gradient, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point)
{
    mandelbrot_colorize(max_iterations, gradient, image, iterations_histogram, results_per_point);

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void supervisor_clear_image(const ImageSize& image_size, sf::Image& image, sf::Texture& texture)
{
    for (int y = 0; y < image_size.height; ++y)
        for (int x = 0; x < image_size.width; ++x)
            image.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), sf::Color::Blue);

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void supervisor_create_work(const SupervisorImageRequest& request, std::vector<int>& combined_iterations_histogram, std::vector<CalculationResult>& results_per_point)
{
    const int size = 100;

    std::lock_guard<std::mutex> lock(mtx);

    for (int y = 0; y < (request.image_size.height / size); ++y)
        for (int x = 0; x < (request.image_size.width / size); ++x)
            worker_message_queue.push(WorkerCalc{request.max_iterations, request.image_size, {x * size, y * size, size}, request.fractal_section, &results_per_point, &combined_iterations_histogram});
}

void supervisor_receive_results(const SupervisorResultsFromWorker& results, sf::Image& image, sf::Texture& texture)
{
    const float max = std::log(static_cast<float>(results.max_iterations));

    for (int y = results.area.start_y; y < (results.area.start_y + results.area.size); ++y) {
        for (int x = results.area.start_x; x < (results.area.start_x + results.area.size); ++x) {
            std::size_t pixel = static_cast<std::size_t>(y * results.image_size.width + x);
            const CalculationResult& rpp{(*results.results_per_point)[pixel]};
            const auto rgb = static_cast<sf::Uint8>(255.0f - 255.0f * std::log(static_cast<float>(rpp.iter)) / max);
            const auto color = sf::Color(rgb, rgb, rgb);
            image.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), color);
            ++pixel;
        }
    }

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void supervisor(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient)
{
    spdlog::debug("supervisor: starting");

    std::vector<int> combined_iterations_histogram(static_cast<std::size_t>(max_iterations + 1));
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(image.getSize().x * image.getSize().y));

    for (unsigned int id = 0; id < num_threads; ++id)
        workers.emplace_back(worker, id, std::ref(mtx), std::ref(cv), std::ref(worker_message_queue), std::ref(supervisor_message_queue));

    while (true) {
        SupervisorMessage msg;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return !supervisor_message_queue.empty(); });

            msg = supervisor_message_queue.front();
            supervisor_message_queue.pop();
        }

        if (std::holds_alternative<SupervisorQuit>(msg)) {
            spdlog::debug("supervisor: received SupervisorQuit");
            break;
        } else if (std::holds_alternative<SupervisorImageRequest>(msg)) {
            SupervisorImageRequest image_request{std::get<SupervisorImageRequest>(msg)};
            supervisor_clear_image(image_request.image_size, image, texture);
            supervisor_create_work(image_request, combined_iterations_histogram, results_per_point);
        } else if (std::holds_alternative<SupervisorResultsFromWorker>(msg)) {
            SupervisorResultsFromWorker results{std::get<SupervisorResultsFromWorker>(msg)};
            supervisor_receive_results(results, image, texture);
        }

        cv.notify_one();
    }

    {
        spdlog::debug("supervisor: signaling workers to stop");
        std::unique_lock<std::mutex> lock(mtx);

        for (int i = 0; i < std::ssize(workers); ++i)
            worker_message_queue.push(WorkerQuit{});
    }

    cv.notify_all();

    spdlog::debug("supervisor: waiting for workers to finish");

    for (auto& t : workers)
        t.join();

    spdlog::debug("supervisor: done");
}

std::future<void> supervisor_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const int max_iterations, const Gradient& gradient)
{
    return std::async(std::launch::async, supervisor, std::ref(image), std::ref(texture), num_threads, max_iterations, gradient);
}

void supervisor_stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    supervisor_message_queue.push(SupervisorQuit{});
    cv.notify_all();
}

void supervisor_calc_image(const SupervisorImageRequest& image_request)
{
    std::lock_guard<std::mutex> lock(mtx);
    supervisor_message_queue.push(image_request);
    cv.notify_all();
}
