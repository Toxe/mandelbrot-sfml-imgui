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
std::condition_variable cv_sv;
std::condition_variable cv_wk;
std::mutex mtx;
std::mutex paint_mtx;

std::queue<SupervisorMessage> supervisor_message_queue;
std::queue<WorkerMessage> worker_message_queue;

std::atomic<Phase> supervisor_phase = Phase::Starting;
int waiting_for_results = 0;

void supervisor_set_phase(const Phase phase)
{
    supervisor_phase = phase;
}

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
    std::lock_guard<std::mutex> lock(mtx);

    for (int y = 0; y < request.image_size.height; y += request.area_size) {
        const int height = std::min(request.image_size.height - y, request.area_size);

        for (int x = 0; x < request.image_size.width; x += request.area_size) {
            const int width = std::min(request.image_size.width - x, request.area_size);
            worker_message_queue.push(WorkerCalc{request.max_iterations, request.image_size, {x, y, width, height}, request.fractal_section, &results_per_point, &combined_iterations_histogram});
        }
    }

    waiting_for_results = static_cast<int>(worker_message_queue.size());
}

sf::Color supervisor_calculation_result_to_color(const CalculationResult& point, const float log_max_iterations)
{
    const auto rgb = static_cast<sf::Uint8>(255.0f - 255.0f * std::log(static_cast<float>(point.iter)) / log_max_iterations);
    return sf::Color(rgb, rgb, rgb);
}

void supervisor_receive_results(const SupervisorResultsFromWorker& results, sf::Image& image, sf::Texture& texture)
{
    const float log_max_iterations = std::log(static_cast<float>(results.max_iterations));

    for (int y = results.area.y; y < (results.area.y + results.area.height); ++y) {
        for (int x = results.area.x; x < (results.area.x + results.area.width); ++x) {
            std::size_t pixel = static_cast<std::size_t>(y * results.image_size.width + x);
            const auto color = supervisor_calculation_result_to_color((*results.results_per_point)[pixel], log_max_iterations);
            image.setPixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), color);
            ++pixel;
        }
    }

    std::lock_guard<std::mutex> lock(paint_mtx);
    texture.update(image);
}

void supervisor_resize_combined_iterations_histogram_if_needed(const SupervisorImageRequest& image_request, std::vector<int>& combined_iterations_histogram)
{
    if (std::ssize(combined_iterations_histogram) != image_request.max_iterations + 1)
        combined_iterations_histogram.resize(static_cast<std::size_t>(image_request.max_iterations + 1));
}

void supervisor_reset_combined_iterations_histogram(std::vector<int>& combined_iterations_histogram)
{
    std::fill(combined_iterations_histogram.begin(), combined_iterations_histogram.end(), 0);
}

SupervisorMessage supervisor_wait_for_message()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv_sv.wait(lock, [&] { return !supervisor_message_queue.empty(); });

    const SupervisorMessage msg = supervisor_message_queue.front();
    supervisor_message_queue.pop();

    return msg;
}

void supervisor(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const Gradient& gradient)
{
    spdlog::debug("supervisor: starting");

    supervisor_set_phase(Phase::Starting);

    std::vector<int> combined_iterations_histogram;
    std::vector<CalculationResult> results_per_point(static_cast<std::size_t>(image.getSize().x * image.getSize().y));

    for (unsigned int id = 0; id < num_threads; ++id)
        workers.emplace_back(worker, id, std::ref(mtx), std::ref(cv_wk), std::ref(cv_sv), std::ref(worker_message_queue), std::ref(supervisor_message_queue));

    supervisor_set_phase(Phase::Idle);

    while (true) {
        const SupervisorMessage msg = supervisor_wait_for_message();

        if (std::holds_alternative<SupervisorQuit>(msg)) {
            spdlog::debug("supervisor: quit");
            supervisor_set_phase(Phase::Shutdown);
            break;
        } else if (std::holds_alternative<SupervisorImageRequest>(msg)) {
            supervisor_set_phase(Phase::Request);
            SupervisorImageRequest image_request{std::get<SupervisorImageRequest>(msg)};
            supervisor_clear_image(image_request.image_size, image, texture);
            supervisor_resize_combined_iterations_histogram_if_needed(image_request, combined_iterations_histogram);
            supervisor_reset_combined_iterations_histogram(combined_iterations_histogram);
            supervisor_create_work(image_request, combined_iterations_histogram, results_per_point);
            supervisor_set_phase(Phase::Waiting);
        } else if (std::holds_alternative<SupervisorResultsFromWorker>(msg)) {
            SupervisorResultsFromWorker results{std::get<SupervisorResultsFromWorker>(msg)};
            supervisor_receive_results(results, image, texture);

            if (--waiting_for_results == 0) {
                supervisor_set_phase(Phase::Coloring);
                supervisor_colorize(image, texture, results.max_iterations, gradient, combined_iterations_histogram, results_per_point);
                supervisor_set_phase(Phase::Idle);
            }
        }

        cv_wk.notify_one();
    }

    {
        spdlog::debug("supervisor: signaling workers to stop");
        std::unique_lock<std::mutex> lock(mtx);

        for (int i = 0; i < std::ssize(workers); ++i)
            worker_message_queue.push(WorkerQuit{});
    }

    cv_wk.notify_all();

    spdlog::debug("supervisor: waiting for workers to finish");

    for (auto& t : workers)
        t.join();

    spdlog::debug("supervisor: stopping");
}

std::future<void> supervisor_start(sf::Image& image, sf::Texture& texture, const unsigned int num_threads, const Gradient& gradient)
{
    return std::async(std::launch::async, supervisor, std::ref(image), std::ref(texture), num_threads, gradient);
}

void supervisor_stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    supervisor_message_queue.push(SupervisorQuit{});
    cv_sv.notify_one();
}

void supervisor_calc_image(const SupervisorImageRequest& image_request)
{
    std::lock_guard<std::mutex> lock(mtx);
    supervisor_message_queue.push(image_request);
    cv_sv.notify_one();
}

const char* supervisor_phase_name(const Phase phase)
{
    switch (phase) {
    case Phase::Starting:
        return "starting";
    case Phase::Idle:
        return "idle";
    case Phase::Request:
        return "request";
    case Phase::Waiting:
        return "waiting";
    case Phase::Coloring:
        return "coloring";
    case Phase::Shutdown:
        return "shutdown";
    default:
        return "unknown";
    }
}
