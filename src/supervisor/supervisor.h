#pragma once

#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

#include "supervisor_status.h"
#include "window.h"
#include "gradient/gradient.h"
#include "messages/message_queue.h"
#include "messages/messages.h"
#include "worker/worker.h"

class App;
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

    std::vector<int> combined_iterations_histogram_;
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

    void resize_and_reset_buffers_if_needed(const ImageSize& image_size, const int max_iterations);

public:
    Supervisor(Window& window);
    ~Supervisor();

    void run(const int num_threads, const Gradient& gradient);
    void join();

    void restart(const int num_threads);
    void shutdown();

    void calculate_image(const SupervisorImageRequest image_request);
    void colorize(const SupervisorColorize colorize);
    void cancel_calculation();

    [[nodiscard]] SupervisorStatus& status() { return status_; };
};