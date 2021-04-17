#include "supervisor.h"

#include <atomic>
#include <cmath>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

#include "message_queue.h"
#include "messages.h"
#include "worker.h"

const sf::Color background_color(0x00, 0x00, 0x20);

std::vector<std::thread> workers;
std::mutex mtx;

MessageQueue<SupervisorMessage> supervisor_message_queue;
MessageQueue<WorkerMessage> worker_message_queue;

std::atomic<Phase> supervisor_phase = Phase::Starting;
int waiting_for_results = 0;

void supervisor_set_phase(const Phase phase)
{
    supervisor_phase = phase;
}

void supervisor_cancel_calc()
{
    // empty the message queue
    waiting_for_results -= worker_message_queue.clear();

    if (waiting_for_results > 0)
        supervisor_set_phase(Phase::Canceled); // wait until all workers have finished
    else
        supervisor_set_phase(Phase::Idle);
}

void supervisor_colorize(const ImageSize& image_size, App& app, const int max_iterations, const Gradient& gradient, const std::vector<int>& iterations_histogram, const std::vector<CalculationResult>& results_per_point)
{
    sf::Image image;
    image.create(static_cast<unsigned int>(image_size.width), static_cast<unsigned int>(image_size.height));

    mandelbrot_colorize(max_iterations, gradient, image, iterations_histogram, results_per_point);
    app.update_texture(image);
}

void supervisor_clear_window(const ImageSize& image_size, App& app)
{
    sf::Image image;
    image.create(static_cast<unsigned int>(image_size.width), static_cast<unsigned int>(image_size.height), background_color);

    if (static_cast<int>(app.texture().getSize().x) != image_size.width || static_cast<int>(app.texture().getSize().y) != image_size.height)
        app.resize_texture(image);
    else
        app.update_texture(image);
}

void supervisor_create_work(const SupervisorImageRequest& request, std::vector<int>& combined_iterations_histogram, std::vector<CalculationResult>& results_per_point)
{
    for (int y = 0; y < request.image_size.height; y += request.area_size) {
        const int height = std::min(request.image_size.height - y, request.area_size);

        for (int x = 0; x < request.image_size.width; x += request.area_size) {
            const int width = std::min(request.image_size.width - x, request.area_size);
            worker_message_queue.send(WorkerCalc{
                request.max_iterations, request.image_size, {x, y, width, height},
                request.fractal_section, &results_per_point, &combined_iterations_histogram,
                std::make_unique<sf::Uint8[]>(static_cast<std::size_t>(4 * width * height))
            });
        }
    }

    waiting_for_results = static_cast<int>(worker_message_queue.size());
}

void supervisor_receive_results(const SupervisorResultsFromWorker& results, App& app)
{
    app.update_texture(results.pixels.get(), results.area);
}

void supervisor_resize_combined_iterations_histogram_if_needed(const SupervisorImageRequest& image_request, std::vector<int>& combined_iterations_histogram)
{
    if (std::ssize(combined_iterations_histogram) != image_request.max_iterations + 1)
        combined_iterations_histogram.resize(static_cast<std::size_t>(image_request.max_iterations + 1));
}

void supervisor_resize_results_per_point_if_needed(const SupervisorImageRequest& image_request, std::vector<CalculationResult>& results_per_point)
{
    if (std::ssize(results_per_point) != (image_request.image_size.width * image_request.image_size.height))
        results_per_point.resize(static_cast<std::size_t>(image_request.image_size.width * image_request.image_size.height));
}

void supervisor_reset_combined_iterations_histogram(std::vector<int>& combined_iterations_histogram)
{
    std::fill(combined_iterations_histogram.begin(), combined_iterations_histogram.end(), 0);
}

void supervisor(App& app, const int num_threads, const Gradient& gradient)
{
    spdlog::debug("supervisor: starting");

    supervisor_set_phase(Phase::Starting);

    std::vector<int> combined_iterations_histogram;
    std::vector<CalculationResult> results_per_point;

    for (int id = 0; id < num_threads; ++id)
        workers.emplace_back(worker, id, std::ref(mtx), std::ref(worker_message_queue), std::ref(supervisor_message_queue));

    supervisor_set_phase(Phase::Idle);

    while (true) {
        SupervisorMessage msg = supervisor_message_queue.wait_for_message();

        if (std::holds_alternative<SupervisorQuit>(msg)) {
            spdlog::debug("supervisor: quit");
            supervisor_set_phase(Phase::Shutdown);
            break;
        } else if (std::holds_alternative<SupervisorImageRequest>(msg)) {
            supervisor_set_phase(Phase::RequestReceived);
            SupervisorImageRequest image_request{std::get<SupervisorImageRequest>(msg)};
            supervisor_clear_window(image_request.image_size, app);
            supervisor_resize_combined_iterations_histogram_if_needed(image_request, combined_iterations_histogram);
            supervisor_resize_results_per_point_if_needed(image_request, results_per_point);
            supervisor_reset_combined_iterations_histogram(combined_iterations_histogram);
            supervisor_create_work(image_request, combined_iterations_histogram, results_per_point);
            supervisor_set_phase(Phase::Waiting);
        } else if (std::holds_alternative<SupervisorResultsFromWorker>(msg)) {
            SupervisorResultsFromWorker results = std::move(std::get<SupervisorResultsFromWorker>(msg));
            supervisor_receive_results(results, app);

            if (--waiting_for_results == 0) {
                if (supervisor_phase != Phase::Canceled) {
                    // if canceled there is no need to colorize the partial image
                    supervisor_set_phase(Phase::Coloring);
                    supervisor_colorize(results.image_size, app, results.max_iterations, gradient, combined_iterations_histogram, results_per_point);
                }

                supervisor_set_phase(Phase::Idle);
            }
        } else if (std::holds_alternative<SupervisorCancel>(msg)) {
            supervisor_cancel_calc();
        }

        worker_message_queue.notify_one();
    }

    spdlog::debug("supervisor: signaling workers to stop");

    for (int i = 0; i < std::ssize(workers); ++i)
        worker_message_queue.send(WorkerQuit{});

    worker_message_queue.notify_all();

    spdlog::debug("supervisor: waiting for workers to finish");

    for (auto& t : workers)
        t.join();

    spdlog::debug("supervisor: stopping");
}

std::future<void> supervisor_start(App& app, const int num_threads, const Gradient& gradient)
{
    return std::async(std::launch::async, supervisor, std::ref(app), num_threads, gradient);
}

void supervisor_shutdown(std::future<void>& supervisor)
{
    supervisor.wait();
}

void supervisor_stop()
{
    supervisor_message_queue.send(SupervisorQuit{});
    supervisor_message_queue.notify_one();
}

void supervisor_calc_image(const SupervisorImageRequest& image_request)
{
    supervisor_message_queue.send(image_request);
    supervisor_message_queue.notify_one();
    supervisor_set_phase(Phase::RequestSent);
}

void supervisor_cancel_render()
{
    supervisor_message_queue.send(SupervisorCancel{});
    supervisor_message_queue.notify_one();
}

const char* supervisor_phase_name(const Phase phase)
{
    switch (phase) {
    case Phase::Starting:
        return "starting";
    case Phase::Idle:
        return "idle";
    case Phase::RequestSent:
        return "request sent";
    case Phase::RequestReceived:
        return "request received";
    case Phase::Waiting:
        return "waiting";
    case Phase::Coloring:
        return "coloring";
    case Phase::Shutdown:
        return "shutdown";
    case Phase::Canceled:
        return "canceled";
    default:
        return "unknown";
    }
}
