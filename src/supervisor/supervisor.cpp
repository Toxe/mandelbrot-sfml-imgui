#include "supervisor.h"

#include <cassert>
#include <cmath>

#include <spdlog/spdlog.h>

#include "command_line/command_line.h"
#include "mandelbrot/mandelbrot.h"

Supervisor::Supervisor(const CommandLine& cli, Window& window)
    : running_{false}, window_{window}, gradient_{load_gradient("benchmark")}
{
    run(cli.num_threads());
}

Supervisor::~Supervisor()
{
    join();
}

void Supervisor::run(const int num_threads)
{
    status_.set_phase(Phase::Starting);

    num_threads_ = num_threads;
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

    auto visitor = [&](auto&& msg) { handle_message(std::move(msg)); };
    running_ = true;

    while (running_)
        std::visit(visitor, supervisor_message_queue_.wait_for_message());

    status_.set_phase(Phase::Shutdown);

    clear_message_queues();
    shutdown_workers();

    spdlog::debug("supervisor: stopping");
}

void Supervisor::restart(const int num_threads)
{
    shutdown();
    run(num_threads);
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

void Supervisor::colorize(const SupervisorColorize colorize)
{
    status_.set_phase(Phase::RequestSent);
    supervisor_message_queue_.send(colorize);
}

void Supervisor::cancel_calculation()
{
    supervisor_message_queue_.send(SupervisorCancel{});
}

void Supervisor::handle_message(SupervisorImageRequest&& image_request)
{
    spdlog::debug("supervisor: received message ImageRequest size: {}x{}, area: {}/{} {}x{}, tile_size: {}",
        image_request.image_size.width, image_request.image_size.height,
        image_request.area.x, image_request.area.y, image_request.area.width, image_request.area.height,
        image_request.tile_size);

    status_.start_calculation(Phase::RequestReceived);

    bool recalculation_needed = resize_and_reset_buffers_if_needed(image_request.image_size, image_request.max_iterations);

    if (recalculation_needed)
        modify_image_request_for_recalculation(image_request);

    if (should_scroll(image_request))
        scroll_results_per_point_array(image_request);

    send_calculation_messages(image_request);

    status_.set_phase(Phase::Calculating);
}

void Supervisor::handle_message(SupervisorCalculationResults&& calculation_results)
{
    spdlog::debug("supervisor: received message CalculationResults area: {}/{} {}x{}", calculation_results.area.x, calculation_results.area.y, calculation_results.area.width, calculation_results.area.height);

    window_.update_texture(calculation_results.pixels.get(), calculation_results.area);

    if (--waiting_for_calculation_results_ == 0) {
        if (status_.phase() != Phase::Canceled) {
            // if canceled there is no need to colorize the partial image
            status_.set_phase(Phase::Coloring);
            build_iterations_histogram();
            equalize_histogram(iterations_histogram_, calculation_results.max_iterations, equalized_iterations_);
            send_colorization_messages(calculation_results.max_iterations, calculation_results.image_size);
        } else {
            status_.stop_calculation(Phase::Idle);
        }
    }

    assert(waiting_for_calculation_results_ >= 0);
}

void Supervisor::handle_message(SupervisorColorizationResults&& colorization_results)
{
    spdlog::debug("supervisor: received message ColorizationResults start_row: {}, num_rows: {}", colorization_results.start_row, colorization_results.num_rows);

    auto data = colorization_results.colorization_buffer->data();
    std::size_t p = static_cast<std::size_t>(4 * (colorization_results.start_row * colorization_results.row_width));
    window_.update_texture(&data[p], CalculationArea{0, colorization_results.start_row, colorization_results.row_width, colorization_results.num_rows});

    if (--waiting_for_colorization_results_ == 0)
        if (status_.phase() != Phase::Canceled)
            status_.stop_calculation(Phase::Idle);

    assert(waiting_for_calculation_results_ >= 0);
}

void Supervisor::handle_message(SupervisorColorize&& colorize)
{
    spdlog::debug("supervisor: received message Colorize");

    gradient_ = colorize.gradient;

    if (results_per_point_.empty()) {
        status_.set_phase(Phase::Idle);
        return;  // we have not calculated an image yet
    }

    status_.start_calculation(Phase::Coloring);
    send_colorization_messages(colorize.max_iterations, colorize.image_size);
}

void Supervisor::handle_message(SupervisorCancel&&)
{
    spdlog::debug("supervisor: received message Cancel");

    // empty the message queue
    waiting_for_calculation_results_ -= worker_message_queue_.clear();

    if (waiting_for_calculation_results_ > 0)
        status_.set_phase(Phase::Canceled); // wait until all workers have finished
    else
        status_.stop_calculation(Phase::Idle);
}

void Supervisor::handle_message(SupervisorQuit&&)
{
    spdlog::debug("supervisor: received message Quit");

    running_ = false;
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
    for (int y = image_request.area.y; y < (image_request.area.y + image_request.area.height); y += image_request.tile_size) {
        const int height = std::min(image_request.area.y + image_request.area.height - y, image_request.tile_size);

        for (int x = image_request.area.x; x < (image_request.area.x + image_request.area.width); x += image_request.tile_size) {
            const int width = std::min(image_request.area.x + image_request.area.width - x, image_request.tile_size);
            worker_message_queue_.send(WorkerCalculate{
                image_request.max_iterations, image_request.image_size, {x, y, width, height},
                image_request.fractal_section, &results_per_point_,
                std::make_unique<sf::Uint8[]>(static_cast<std::size_t>(4 * width * height))
            });

            ++waiting_for_calculation_results_;
        }
    }

    spdlog::trace("supervisor: sent {} Calculate messages", waiting_for_calculation_results_);
}

void Supervisor::send_colorization_messages(const int max_iterations, const ImageSize& image_size)
{
    const int min_rows_per_thread = image_size.height / static_cast<int>(std::ssize(workers_));
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
            &results_per_point_, &equalized_iterations_, &colorization_buffer_
        });

        ++waiting_for_colorization_results_;
    }

    spdlog::trace("supervisor: sent {} Colorize messages", waiting_for_colorization_results_);
}

bool Supervisor::resize_and_reset_buffers_if_needed(const ImageSize& image_size, const int max_iterations)
{
    bool recalculation_needed = false;

    if (std::ssize(results_per_point_) != (image_size.width * image_size.height) || std::ssize(colorization_buffer_) != (4 * image_size.width * image_size.height)) {
        results_per_point_.resize(static_cast<std::size_t>(image_size.width * image_size.height));
        colorization_buffer_.resize(static_cast<std::size_t>(4 * image_size.width * image_size.height));
        recalculation_needed = true;
    }

    if (std::ssize(iterations_histogram_) != max_iterations + 1 || std::ssize(equalized_iterations_) != max_iterations + 1) {
        iterations_histogram_.resize(static_cast<std::size_t>(max_iterations + 1));
        equalized_iterations_.resize(static_cast<std::size_t>(max_iterations + 1));
        recalculation_needed = true;
    }

    const auto render_buffer_size = render_buffer_.getSize();

    if (static_cast<int>(render_buffer_size.x) != image_size.width || static_cast<int>(render_buffer_size.y) != image_size.height) {
        render_buffer_.create(static_cast<unsigned int>(image_size.width), static_cast<unsigned int>(image_size.height), background_color_);
        window_.resize_texture(render_buffer_);
        recalculation_needed = true;
    }

    return recalculation_needed;
}

void Supervisor::modify_image_request_for_recalculation(SupervisorImageRequest& image_request) const
{
    image_request.scroll = Scroll{0, 0};
    image_request.area = CalculationArea{0, 0, image_request.image_size.width, image_request.image_size.height};
}

[[nodiscard]] bool Supervisor::should_scroll(const SupervisorImageRequest& image_request) const
{
    return image_request.scroll.x != 0 || image_request.scroll.y != 0;
}

void Supervisor::scroll_results_per_point_array(const SupervisorImageRequest& image_request)
{
    assert(image_request.scroll.x != 0 || image_request.scroll.y != 0);

    const int dx = image_request.scroll.x;
    const int dy = image_request.scroll.y;

    if (dy < 0) {
        for (int y = image_request.image_size.height - 1; y >= 0; --y)
            copy_results_per_point_row(dx, dy, image_request.image_size.width, image_request.image_size.height, y);
    } else {
        for (int y = 0; y < image_request.image_size.height; ++y)
            copy_results_per_point_row(dx, dy, image_request.image_size.width, image_request.image_size.height, y);
    }
}

void Supervisor::copy_results_per_point_row(const int dx, const int dy, const int image_width, const int image_height, const int y)
{
    if (dx < 0) {
        for (int x = image_width - 1; x >= 0; --x) {
            const int dstx = x - dx;
            const int dsty = y - dy;

            if (dstx >= 0 && dstx < image_width && dsty >= 0 && dsty < image_height) {
                auto src = results_per_point_.begin() + (y * image_width + x);
                auto dst = results_per_point_.begin() + (dsty * image_width + dstx);
                *dst = *src;
            }
        }
    } else {
        for (int x = 0; x < image_width; ++x) {
            const int dstx = x - dx;
            const int dsty = y - dy;

            if (dstx >= 0 && dstx < image_width && dsty >= 0 && dsty < image_height) {
                auto src = results_per_point_.begin() + (y * image_width + x);
                auto dst = results_per_point_.begin() + (dsty * image_width + dstx);
                *dst = *src;
            }
        }
    }
}

void Supervisor::build_iterations_histogram()
{
    // set histogram back to 0
    std::fill(iterations_histogram_.begin(), iterations_histogram_.end(), 0);

    for (const auto& point : results_per_point_)
        ++iterations_histogram_[point.iter];

    // [max_iterations] must be zero (as we do not count the iterations of the points inside the Mandelbrot Set)
    iterations_histogram_.back() = 0;
}
