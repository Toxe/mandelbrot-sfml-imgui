#include "supervisor.h"

#include <cassert>
#include <cmath>

#include <spdlog/spdlog.h>

#include "mandelbrot.h"

Supervisor::Supervisor(Window& window)
    : window_{window}
{
}

Supervisor::~Supervisor()
{
    join();
}

void Supervisor::run(const int num_threads, const Gradient& gradient)
{
    status_.set_phase(Phase::Starting);

    num_threads_ = num_threads;
    gradient_ = gradient;

    thread_ = std::thread(&Supervisor::main, this);
}

void Supervisor::join()
{
    if (thread_.joinable())
        thread_.join();
}

void Supervisor::main()
{
    spdlog::debug("supervisor: starting");

    start_workers();
    status_.set_phase(Phase::Idle);

    while (handle_message(supervisor_message_queue_.wait_for_message()))
        ;

    status_.set_phase(Phase::Shutdown);

    clear_message_queues();
    shutdown_workers();

    spdlog::debug("supervisor: stopping");
}

void Supervisor::restart(const int num_threads)
{
    shutdown();
    run(num_threads, gradient_);
}

void Supervisor::shutdown()
{
    supervisor_message_queue_.send(SupervisorQuit{});
    join();
}

void Supervisor::calculate_image(const SupervisorImageRequest image_request)
{
    status_.set_phase(Phase::RequestSent);
    supervisor_message_queue_.send(image_request);
}

void Supervisor::cancel_calculation()
{
    supervisor_message_queue_.send(SupervisorCancel{});
}

[[nodiscard]] bool Supervisor::handle_message(SupervisorMessage msg)
{
    if (std::holds_alternative<SupervisorQuit>(msg))
        return false;

    if (std::holds_alternative<SupervisorImageRequest>(msg))
        handle_image_request_message(std::move(std::get<SupervisorImageRequest>(msg)));
    else if (std::holds_alternative<SupervisorCalculationResults>(msg))
        handle_calculation_results_message(std::move(std::get<SupervisorCalculationResults>(msg)));
    else if (std::holds_alternative<SupervisorColorizationResults>(msg))
        handle_colorization_results_message(std::move(std::get<SupervisorColorizationResults>(msg)));
    else if (std::holds_alternative<SupervisorCancel>(msg))
        handle_cancel_message(std::move(std::get<SupervisorCancel>(msg)));

    return true;
}

void Supervisor::handle_image_request_message(SupervisorImageRequest image_request)
{
    status_.start_calculation(Phase::RequestReceived);
    resize_and_reset_buffers_if_needed(image_request);
    send_calculation_messages(image_request);
    status_.set_phase(Phase::Waiting);
}

void Supervisor::handle_calculation_results_message(SupervisorCalculationResults calculation_results)
{
    window_.update_texture(calculation_results.pixels.get(), calculation_results.area);

    if (--waiting_for_calculation_results_ == 0) {
        if (status_.phase() != Phase::Canceled) {
            // if canceled there is no need to colorize the partial image
            status_.set_phase(Phase::Coloring);
            equalize_histogram(combined_iterations_histogram_, calculation_results.max_iterations, equalized_iterations_);
            send_colorization_messages(calculation_results.max_iterations, calculation_results.image_size);
        } else {
            status_.stop_calculation(Phase::Idle);
        }
    }

    assert(waiting_for_calculation_results_ >= 0);
}

void Supervisor::handle_colorization_results_message(SupervisorColorizationResults colorization_results)
{
    auto data = colorization_results.colorization_buffer->data();
    std::size_t p = static_cast<std::size_t>(4 * (colorization_results.start_row * colorization_results.row_width));
    window_.update_texture(&data[p], CalculationArea{0, colorization_results.start_row, colorization_results.row_width, colorization_results.num_rows});

    if (--waiting_for_colorization_results_ == 0)
        if (status_.phase() != Phase::Canceled)
            status_.stop_calculation(Phase::Idle);

    assert(waiting_for_calculation_results_ >= 0);
}

void Supervisor::handle_cancel_message(SupervisorCancel)
{
    // empty the message queue
    waiting_for_calculation_results_ -= worker_message_queue_.clear();

    if (waiting_for_calculation_results_ > 0)
        status_.set_phase(Phase::Canceled); // wait until all workers have finished
    else
        status_.stop_calculation(Phase::Idle);
}

void Supervisor::start_workers()
{
    spdlog::debug("supervisor: starting workers");

    workers_.reserve(static_cast<std::size_t>(num_threads_));

    for (int id = 0; id < num_threads_; ++id) {
        workers_.emplace_back(id, worker_message_queue_, supervisor_message_queue_);
        workers_.back().run();
    }
}

void Supervisor::shutdown_workers()
{
    spdlog::debug("supervisor: signaling workers to stop");

    for (int i = 0; i < std::ssize(workers_); ++i)
        worker_message_queue_.send(WorkerQuit{});

    spdlog::debug("supervisor: waiting for workers to finish");

    for (auto& w : workers_)
        w.join();

    workers_.clear();
}

void Supervisor::clear_message_queues()
{
    supervisor_message_queue_.clear();
    worker_message_queue_.clear();

    waiting_for_calculation_results_ = 0;
    waiting_for_colorization_results_ = 0;
}

void Supervisor::send_calculation_messages(const SupervisorImageRequest& image_request)
{
    for (int y = 0; y < image_request.image_size.height; y += image_request.area_size) {
        const int height = std::min(image_request.image_size.height - y, image_request.area_size);

        for (int x = 0; x < image_request.image_size.width; x += image_request.area_size) {
            const int width = std::min(image_request.image_size.width - x, image_request.area_size);
            worker_message_queue_.send(WorkerCalculate{
                image_request.max_iterations, image_request.image_size, {x, y, width, height},
                image_request.fractal_section, &results_per_point_, &combined_iterations_histogram_,
                std::make_unique<sf::Uint8[]>(static_cast<std::size_t>(4 * width * height))
            });

            ++waiting_for_calculation_results_;
        }
    }
}

void Supervisor::send_colorization_messages(const int max_iterations, const ImageSize& image_size)
{
    const int min_rows_per_thread = image_size.height / std::ssize(workers_);
    int extra_rows = image_size.height % std::ssize(workers_);
    int next_start_row = 0;

    for (int i = 0; i < std::ssize(workers_); ++i) {
        const int start_row = next_start_row;
        int num_rows = min_rows_per_thread;

        if (extra_rows > 0) {
            ++num_rows;
            --extra_rows;
        }

        next_start_row = start_row + num_rows;

        worker_message_queue_.send(WorkerColorize{
            max_iterations, start_row, num_rows, image_size.width, &gradient_,
            &combined_iterations_histogram_, &results_per_point_, &equalized_iterations_, &colorization_buffer_
        });

        ++waiting_for_colorization_results_;
    }
}

void Supervisor::resize_and_reset_buffers_if_needed(const SupervisorImageRequest& image_request)
{
    if (std::ssize(results_per_point_) != (image_request.image_size.width * image_request.image_size.height))
        results_per_point_.resize(static_cast<std::size_t>(image_request.image_size.width * image_request.image_size.height));

    if (std::ssize(equalized_iterations_) != image_request.max_iterations + 1)
        equalized_iterations_.resize(static_cast<std::size_t>(image_request.max_iterations + 1));

    if (std::ssize(colorization_buffer_) != (4 * image_request.image_size.width * image_request.image_size.height))
        colorization_buffer_.resize(static_cast<std::size_t>(4 * image_request.image_size.width * image_request.image_size.height));

    if (std::ssize(combined_iterations_histogram_) != image_request.max_iterations + 1)
        combined_iterations_histogram_.resize(static_cast<std::size_t>(image_request.max_iterations + 1));

    // set histogram back to 0
    std::fill(combined_iterations_histogram_.begin(), combined_iterations_histogram_.end(), 0);

    // update render buffer and window texture
    render_buffer_.create(static_cast<unsigned int>(image_request.image_size.width), static_cast<unsigned int>(image_request.image_size.height), background_color_);

    if (window_.texture().getSize() != render_buffer_.getSize())
        window_.resize_texture(render_buffer_);
    else
        window_.update_texture(render_buffer_);
}
