#include "supervisor.h"

#include <atomic>
#include <cmath>
#include <future>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

#include "app.h"
#include "mandelbrot.h"
#include "message_queue.h"
#include "messages.h"
#include "worker.h"

const sf::Color background_color(0x00, 0x00, 0x20);

std::vector<std::thread> workers;

MessageQueue<SupervisorMessage> supervisor_message_queue;
MessageQueue<WorkerMessage> worker_message_queue;

std::atomic<Phase> supervisor_phase = Phase::Starting;
int waiting_for_results = 0;
int waiting_for_colorization_results = 0;

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

void supervisor_equalize_combined_iterations_histogram(const int max_iterations, const std::vector<int>& combined_iterations_histogram, std::vector<float>& equalized_iterations)
{
    equalize_histogram(combined_iterations_histogram, max_iterations, equalized_iterations);
}

void supervisor_resize_and_clear_render_image(const ImageSize& image_size, sf::Image& render_image)
{
    render_image.create(static_cast<unsigned int>(image_size.width), static_cast<unsigned int>(image_size.height), background_color);
}

void supervisor_update_texture(const sf::Image& render_image, App& app)
{
    if (app.texture().getSize() != render_image.getSize())
        app.resize_texture(render_image);
    else
        app.update_texture(render_image);
}

void supervisor_create_work(const SupervisorImageRequest& request, std::vector<int>& combined_iterations_histogram, std::vector<CalculationResult>& results_per_point)
{
    for (int y = 0; y < request.image_size.height; y += request.area_size) {
        const int height = std::min(request.image_size.height - y, request.area_size);

        for (int x = 0; x < request.image_size.width; x += request.area_size) {
            const int width = std::min(request.image_size.width - x, request.area_size);
            worker_message_queue.send(WorkerCalculate{
                request.max_iterations, request.image_size, {x, y, width, height},
                request.fractal_section, &results_per_point, &combined_iterations_histogram,
                std::make_unique<sf::Uint8[]>(static_cast<std::size_t>(4 * width * height))
            });

            ++waiting_for_results;
        }
    }
}

void supervisor_create_colorization_requests(const int max_iterations, const ImageSize& image_size, const Gradient& gradient,
    const std::vector<int>& combined_iterations_histogram, const std::vector<CalculationResult>& results_per_point,
    const std::vector<float>& equalized_iterations, std::vector<sf::Uint8>& colorization_buffer)
{
    const int min_rows_per_thread = image_size.height / std::ssize(workers);
    int extra_rows = image_size.height % std::ssize(workers);
    int next_start_row = 0;

    for (int i = 0; i < std::ssize(workers); ++i) {
        const int start_row = next_start_row;
        int num_rows = min_rows_per_thread;

        if (extra_rows > 0) {
            ++num_rows;
            --extra_rows;
        }

        next_start_row = start_row + num_rows;

        worker_message_queue.send(WorkerColorize{
            max_iterations, image_size, {0, start_row, image_size.width, num_rows}, &gradient,
            &combined_iterations_histogram, &results_per_point, &equalized_iterations, &colorization_buffer
        });

        ++waiting_for_colorization_results;
    }
}

void supervisor_receive_results(const SupervisorCalculationResults& calculation_results, App& app)
{
    app.update_texture(calculation_results.pixels.get(), calculation_results.area);
}

void supervisor_receive_colorization_results(const SupervisorColorizationResults& colorization_results, App& app)
{
    auto data = colorization_results.colorization_buffer->data();
    std::size_t p = static_cast<std::size_t>(4 * (colorization_results.area.y * colorization_results.area.width + colorization_results.area.x));
    app.update_texture(&data[p], colorization_results.area);
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

void supervisor_resize_equalized_iterations_if_needed(const SupervisorImageRequest& image_request, std::vector<float>& equalized_iterations)
{
    if (std::ssize(equalized_iterations) != image_request.max_iterations + 1)
        equalized_iterations.resize(static_cast<std::size_t>(image_request.max_iterations + 1));
}

void supervisor_resize_colorization_buffer_if_needed(const SupervisorImageRequest& image_request, std::vector<sf::Uint8>& colorization_buffer)
{
    if (std::ssize(colorization_buffer) != (4 * image_request.image_size.width * image_request.image_size.height))
        colorization_buffer.resize(static_cast<std::size_t>(4 * image_request.image_size.width * image_request.image_size.height));
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
    std::vector<float> equalized_iterations;
    std::vector<sf::Uint8> colorization_buffer;
    sf::Image render_image;

    for (int id = 0; id < num_threads; ++id)
        workers.emplace_back(worker, id, std::ref(worker_message_queue), std::ref(supervisor_message_queue));

    supervisor_set_phase(Phase::Idle);

    while (true) {
        SupervisorMessage msg = supervisor_message_queue.wait_for_message();

        if (std::holds_alternative<SupervisorQuit>(msg)) {
            supervisor_set_phase(Phase::Shutdown);
            break;
        } else if (std::holds_alternative<SupervisorImageRequest>(msg)) {
            supervisor_set_phase(Phase::RequestReceived);
            SupervisorImageRequest image_request{std::get<SupervisorImageRequest>(msg)};
            supervisor_resize_and_clear_render_image(image_request.image_size, render_image);
            supervisor_update_texture(render_image, app);
            supervisor_resize_combined_iterations_histogram_if_needed(image_request, combined_iterations_histogram);
            supervisor_resize_results_per_point_if_needed(image_request, results_per_point);
            supervisor_resize_equalized_iterations_if_needed(image_request, equalized_iterations);
            supervisor_resize_colorization_buffer_if_needed(image_request, colorization_buffer);
            supervisor_reset_combined_iterations_histogram(combined_iterations_histogram);
            supervisor_create_work(image_request, combined_iterations_histogram, results_per_point);
            supervisor_set_phase(Phase::Waiting);
        } else if (std::holds_alternative<SupervisorCalculationResults>(msg)) {
            SupervisorCalculationResults calculation_results = std::move(std::get<SupervisorCalculationResults>(msg));
            supervisor_receive_results(calculation_results, app);

            if (--waiting_for_results == 0) {
                if (supervisor_phase != Phase::Canceled) {
                    // if canceled there is no need to colorize the partial image
                    supervisor_set_phase(Phase::Coloring);
                    supervisor_equalize_combined_iterations_histogram(calculation_results.max_iterations, combined_iterations_histogram, equalized_iterations);
                    supervisor_create_colorization_requests(calculation_results.max_iterations, calculation_results.image_size, gradient, combined_iterations_histogram, results_per_point, equalized_iterations, colorization_buffer);
                }
            }
        } else if (std::holds_alternative<SupervisorColorizationResults>(msg)) {
            SupervisorColorizationResults colorization_results = std::move(std::get<SupervisorColorizationResults>(msg));
            supervisor_receive_colorization_results(colorization_results, app);

            if (--waiting_for_colorization_results == 0) {
                if (supervisor_phase != Phase::Canceled) {
                    supervisor_set_phase(Phase::Idle);
                }
            }
        } else if (std::holds_alternative<SupervisorCancel>(msg)) {
            supervisor_cancel_calc();
        }
    }

    spdlog::debug("supervisor: signaling workers to stop");

    for (int i = 0; i < std::ssize(workers); ++i)
        worker_message_queue.send(WorkerQuit{});

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
}

void supervisor_calc_image(const SupervisorImageRequest& image_request)
{
    supervisor_message_queue.send(image_request);
    supervisor_set_phase(Phase::RequestSent);
}

void supervisor_cancel_render()
{
    supervisor_message_queue.send(SupervisorCancel{});
}

Phase supervisor_get_phase()
{
    return supervisor_phase;
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
