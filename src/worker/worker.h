#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include <SFML/Config.hpp>

#include "messages/message_queue.h"
#include "messages/messages.h"

class Worker {
    inline static std::mutex mtx_;

    const int id_;
    bool running_;

    std::thread thread_;

    MessageQueue<WorkerMessage>& worker_message_queue_;
    MessageQueue<SupervisorMessage>& supervisor_message_queue_;

    void main();

    void handle_message(WorkerCalculate&& calculate);
    void handle_message(WorkerColorize&& colorize);
    void handle_message(WorkerQuit&&);

    [[nodiscard]] sf::Uint8 calculation_result_to_grayscale(const CalculationResult& point, const float log_max_iterations);
    void draw_pixels(const WorkerCalculate& calculate);

public:
    Worker(const int id, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue);
    Worker(Worker&& other);
    ~Worker();

    void run();
    void join();
};
