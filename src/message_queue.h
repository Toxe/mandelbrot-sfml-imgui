#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "util/mutex_timer.h"

extern MutexTimer mutex_timer_message_queue_send;
extern MutexTimer mutex_timer_message_queue_wait_for_message;
extern MutexTimer mutex_timer_message_queue_clear;
extern MutexTimer mutex_timer_message_queue_empty;
extern MutexTimer mutex_timer_message_queue_size;

template <typename T>
class MessageQueue {
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<T> queue_;

public:
    void send(T&& msg);

    [[nodiscard]] T wait_for_message();

    void notify_one() { cv_.notify_one(); };
    void notify_all() { cv_.notify_all(); };

    int clear();
    [[nodiscard]] bool empty();
    [[nodiscard]] auto size();
};

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    const auto t0 = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mtx_);
    mutex_timer_message_queue_wait_for_message.update(t0);
    queue_.emplace(std::move(msg));
}

template <typename T>
[[nodiscard]] T MessageQueue<T>::wait_for_message()
{
    const auto t0 = std::chrono::high_resolution_clock::now();
    std::unique_lock<std::mutex> lock(mtx_);
    mutex_timer_message_queue_send.update(t0);
    cv_.wait(lock, [&] { return !queue_.empty(); });

    T msg = std::move(queue_.front());
    queue_.pop();

    return msg;
}

template <typename T>
int MessageQueue<T>::clear()
{
    const auto t0 = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mtx_);
    mutex_timer_message_queue_clear.update(t0);
    int messages_removed = 0;

    while (!queue_.empty()) {
        queue_.pop();
        --messages_removed;
    }

    return messages_removed;
}

template <typename T>
[[nodiscard]] bool MessageQueue<T>::empty()
{
    const auto t0 = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mtx_);
    mutex_timer_message_queue_empty.update(t0);
    return queue_.empty();
}

template <typename T>
[[nodiscard]] auto MessageQueue<T>::size()
{
    const auto t0 = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mtx_);
    mutex_timer_message_queue_size.update(t0);
    return queue_.size();
}
