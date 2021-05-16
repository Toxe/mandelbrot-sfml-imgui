#pragma once

#include <thread>
#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include "supervisor_status.h"
#include "gradient/gradient.h"
#include "messages/message_queue.h"
#include "messages/messages.h"
#include "window/window.h"
#include "worker/worker.h"

class App;
class CommandLine;
struct SupervisorImageRequest;

class Supervisor {
    const sf::Color background_color_ = sf::Color{0x00, 0x00, 0x20};

    bool running_;
    SupervisorStatus status_;

    int num_threads_;
    std::thread thread_;

    std::vector<Worker> workers_;

    Window& window_;

    Gradient gradient_;

    MessageQueue<WorkerMessage> worker_message_queue_;
    MessageQueue<SupervisorMessage> supervisor_message_queue_;

    int waiting_for_calculation_results_ = 0;
    int waiting_for_colorization_results_ = 0;

    std::vector<int> iterations_histogram_;
    std::vector<CalculationResult> results_per_point_;
    std::vector<float> equalized_iterations_;
    std::vector<sf::Uint8> colorization_buffer_;
    sf::Image render_buffer_;

    void main();

    void handle_message(SupervisorImageRequest&& image_request);
    void handle_message(SupervisorCalculationResults&& calculation_results);
    void handle_message(SupervisorColorizationResults&& colorization_results);
    void handle_message(SupervisorColorize&& colorize);
    void handle_message(SupervisorCancel&&);
    void handle_message(SupervisorQuit&&);

    void start_workers();
    void shutdown_workers();
    void clear_message_queues();

    void send_calculation_messages(const SupervisorImageRequest& image_request);
    void send_colorization_messages(const int max_iterations, const ImageSize& image_size);

    bool resize_and_reset_buffers_if_needed(const ImageSize& image_size, const int max_iterations);
    void build_iterations_histogram();

    void modify_image_request_for_recalculation(SupervisorImageRequest& image_request) const;

    [[nodiscard]] bool should_scroll(const SupervisorImageRequest& image_request) const;
    void scroll_results_per_point_array(const SupervisorImageRequest& image_request);
    void copy_results_per_point_row(const int dx, const int dy, const int image_width, const int image_height, const int y);

public:
    Supervisor(const CommandLine& cli, Window& window);
    ~Supervisor();

    void run(const int num_threads);
    void join();

    void restart(const int num_threads);
    void shutdown();

    void calculate_image(const SupervisorImageRequest image_request);
    void colorize(const SupervisorColorize colorize);
    void cancel_calculation();

    [[nodiscard]] SupervisorStatus& status() { return status_; };
};
