#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include <SFML/Config.hpp>

#include "message_queue.h"
#include "messages.h"

class Worker {
    inline static std::mutex mtx_;

    const int id_;
    bool running_;

    std::thread thread_;

    MessageQueue<WorkerMessage>& worker_message_queue_;
    MessageQueue<SupervisorMessage>& supervisor_message_queue_;

    std::vector<int> iterations_histogram_;

    void main();

    void handle_message(WorkerCalculate&& calculate);
    void handle_message(WorkerColorize&& colorize);
    void handle_message(WorkerQuit&&);

    void resize_iterations_histogram_if_needed(const WorkerCalculate& calculate);
    void combine_iterations_histogram(std::vector<int>& combined_iterations_histogram);

    [[nodiscard]] sf::Uint8 calculation_result_to_grayscale(const CalculationResult& point, const float log_max_iterations);
    void draw_pixels(const WorkerCalculate& calculate);

public:
    Worker(const int id, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue);
    Worker(Worker&& other);
    ~Worker();

    void run();
    void join();
};
