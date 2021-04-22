#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

#include "gradient.h"
#include "message_queue.h"
#include "messages.h"
#include "phase.h"
#include "window.h"
#include "worker.h"

class App;
struct SupervisorImageRequest;

class Supervisor {
    const sf::Color background_color_ = sf::Color{0x00, 0x00, 0x20};

    std::atomic<Phase> phase_ = Phase::Starting;

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

    [[nodiscard]] bool handle_message(SupervisorMessage msg);
    void handle_image_request_message(SupervisorImageRequest image_request);
    void handle_calculation_results_message(SupervisorCalculationResults calculation_results);
    void handle_colorization_results_message(SupervisorColorizationResults colorization_results);
    void handle_cancel_message(SupervisorCancel);

    void start_workers();
    void shutdown_workers();
    void clear_message_queues();

    void set_phase(const Phase phase);

    void send_calculation_messages(const SupervisorImageRequest& image_request);
    void send_colorization_messages(const int max_iterations, const ImageSize& image_size);

    void resize_and_reset_buffers_if_needed(const SupervisorImageRequest& image_request);

public:
    Supervisor(Window& window);
    ~Supervisor();

    void run(const int num_threads, const Gradient& gradient);
    void join();

    void restart(const int num_threads);
    void shutdown();

    void calculate_image(const SupervisorImageRequest image_request);
    void cancel_calculation();

    [[nodiscard]] Phase get_phase() const { return phase_; };
};
