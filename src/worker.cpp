#include "worker.h"

#include <spdlog/spdlog.h>

#include "mandelbrot.h"

Worker::Worker(const int id, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue) :
    id_{id}, worker_message_queue_{worker_message_queue}, supervisor_message_queue_{supervisor_message_queue}
{
}

Worker::Worker(Worker&& other) :
    id_{other.id_}, thread_{std::move(other.thread_)},
    worker_message_queue_{other.worker_message_queue_}, supervisor_message_queue_{other.supervisor_message_queue_},
    iterations_histogram_{std::move(other.iterations_histogram_)}
{
}

Worker::~Worker()
{
    join();
}

void Worker::run()
{
    thread_ = std::thread(&Worker::main, this);
}

void Worker::join()
{
    if (thread_.joinable())
        thread_.join();
}

void Worker::main()
{
    spdlog::debug("worker {}: started", id_);

    while (handle_message(worker_message_queue_.wait_for_message()))
        ;

    spdlog::debug("worker {}: stopping", id_);
}

[[nodiscard]] bool Worker::handle_message(WorkerMessage msg)
{
    if (std::holds_alternative<WorkerQuit>(msg))
        return false;

    if (std::holds_alternative<WorkerCalculate>(msg))
        handle_calculate_message(std::move(std::get<WorkerCalculate>(msg)));
    else if (std::holds_alternative<WorkerColorize>(msg))
        handle_colorize_message(std::move(std::get<WorkerColorize>(msg)));

    return true;
}

void Worker::handle_calculate_message(WorkerCalculate calculate)
{
    resize_iterations_histogram_if_needed(calculate);
    mandelbrot_calc(calculate.image_size, calculate.fractal_section, calculate.max_iterations, iterations_histogram_, *calculate.results_per_point, calculate.area);
    draw_pixels(calculate);

    {
        std::lock_guard<std::mutex> lock(mtx_);
        combine_iterations_histogram(*calculate.combined_iterations_histogram);
    }

    supervisor_message_queue_.send(SupervisorCalculationResults{calculate.max_iterations, calculate.image_size, calculate.area, calculate.fractal_section, calculate.results_per_point, std::move(calculate.pixels)});
}

void Worker::handle_colorize_message(WorkerColorize colorize)
{
    mandelbrot_colorize(colorize);
    supervisor_message_queue_.send(SupervisorColorizationResults{colorize.start_row, colorize.num_rows, colorize.row_width, colorize.colorization_buffer});
}

void Worker::resize_iterations_histogram_if_needed(const WorkerCalculate& calculate)
{
    if (std::ssize(iterations_histogram_) != calculate.max_iterations + 1)
        iterations_histogram_.resize(static_cast<std::size_t>(calculate.max_iterations + 1));
}

void Worker::combine_iterations_histogram(std::vector<int>& combined_iterations_histogram)
{
    std::transform(iterations_histogram_.cbegin(), iterations_histogram_.cend(),
        combined_iterations_histogram.cbegin(), combined_iterations_histogram.begin(), std::plus<>{});
}

[[nodiscard]] sf::Uint8 Worker::calculation_result_to_grayscale(const CalculationResult& point, const float log_max_iterations)
{
    return static_cast<sf::Uint8>(255.0f - 255.0f * std::log(static_cast<float>(point.iter)) / log_max_iterations);
}

void Worker::draw_pixels(const WorkerCalculate& calculate)
{
    const float log_max_iterations = std::log(static_cast<float>(calculate.max_iterations));
    auto p = calculate.pixels.get();

    for (int y = 0; y < calculate.area.height; ++y) {
        for (int x = 0; x < calculate.area.width; ++x) {
            const std::size_t point = static_cast<std::size_t>((y + calculate.area.y) * calculate.image_size.width + (x + calculate.area.x));
            const auto grey = calculation_result_to_grayscale((*calculate.results_per_point)[point], log_max_iterations);
            *p++ = grey;
            *p++ = grey;
            *p++ = grey;
            *p++ = 255;
        }
    }
}
